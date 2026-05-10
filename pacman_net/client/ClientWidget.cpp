#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
#endif

#include "ClientWidget.h"
#include <QPainter>
#include <stdexcept>
#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

// ── SimpleAMQPHandler implementation ─────────────────────────────────────────

SimpleAMQPHandler::SimpleAMQPHandler(const std::string &host, uint16_t port)
    : m_recvBuf(65536)
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    // Use getaddrinfo so both hostnames ("localhost") and IPs ("127.0.0.1") work
    struct addrinfo hints{};
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *result = nullptr;
    std::string portStr = std::to_string(port);
    int ret = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
    if (ret != 0 || result == nullptr)
        throw std::runtime_error("Cannot resolve host: " + host);

#ifdef _WIN32
    m_sock = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
#else
    m_sock = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
#endif

    if (::connect(m_sock, result->ai_addr, (int)result->ai_addrlen) != 0) {
        freeaddrinfo(result);
        throw std::runtime_error("Cannot connect to RabbitMQ at " + host +
                                 ":" + portStr);
    }
    freeaddrinfo(result);
}

SimpleAMQPHandler::~SimpleAMQPHandler()
{
#ifdef _WIN32
    closesocket(m_sock);
    WSACleanup();
#else
    ::close(m_sock);
#endif
}

void SimpleAMQPHandler::onData(AMQP::Connection *, const char *data, size_t size)
{
    size_t sent = 0;
    while (sent < size) {
        int n = ::send(m_sock,
                       data + sent,
                       (int)(size - sent), 0);
        if (n <= 0) break;
        sent += n;
    }
}

void SimpleAMQPHandler::onError(AMQP::Connection *, const char *msg)
{
    std::cerr << "AMQP error: " << msg << "\n";
    m_closed = true;
}

void SimpleAMQPHandler::process(AMQP::Connection *conn)
{
    // Non-blocking peek: check if data is available
#ifdef _WIN32
    u_long available = 0;
    ioctlsocket(m_sock, FIONREAD, &available);
#else
    int available = 0;
    ioctl(m_sock, FIONREAD, &available);
#endif
    if (available <= 0) return;

    int n = ::recv(m_sock, m_recvBuf.data(),
                   (int)std::min((size_t)available, m_recvBuf.size()), 0);
    if (n > 0)
        conn->parse(m_recvBuf.data(), n);
}

// ── Helper: create handler + connection + wait until ready ───────────────────

static void waitReady(SimpleAMQPHandler *handler, AMQP::Connection *conn)
{
    // Process until AMQP handshake completes (onReady fires)
    for (int i = 0; i < 200 && !handler->ready(); i++) {
        handler->process(conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!handler->ready())
        throw std::runtime_error("AMQP connection did not become ready");
}

// ── Constructor / Destructor ──────────────────────────────────────────────────

ClientWidget::ClientWidget(const std::string &host, int port,
                           const std::string &user, const std::string &password,
                           QWidget *parent)
    : QOpenGLWidget(parent),
      m_host(host), m_port(port), m_user(user), m_password(password)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    memset(m_list, 0, sizeof(m_list));

    setupSubscriber();
    setupPublisher();

    m_running  = true;
    m_receiver = std::thread(&ClientWidget::subscriberLoop, this);

    m_redrawTimer = new QTimer(this);
    connect(m_redrawTimer, &QTimer::timeout, this, &ClientWidget::onRedraw);
    m_redrawTimer->start(16);

    setFocusPolicy(Qt::StrongFocus);
    resize(1200, 780);
}

ClientWidget::~ClientWidget()
{
    m_running = false;
    if (m_receiver.joinable()) m_receiver.join();

    delete m_sub_chan; delete m_sub_conn; delete m_sub_handler;
    delete m_pub_chan; delete m_pub_conn; delete m_pub_handler;

    google::protobuf::ShutdownProtobufLibrary();
}

// ── AMQP connections ──────────────────────────────────────────────────────────

void ClientWidget::setupSubscriber()
{
    m_sub_handler = new SimpleAMQPHandler(m_host, (uint16_t)m_port);
    m_sub_conn    = new AMQP::Connection(m_sub_handler,
                        AMQP::Login(m_user, m_password), "/");
    waitReady(m_sub_handler, m_sub_conn);

    m_sub_chan = new AMQP::Channel(m_sub_conn);
    m_sub_chan->declareExchange("game.state", AMQP::fanout, AMQP::durable);
    m_sub_chan->declareQueue(AMQP::exclusive | AMQP::autodelete)
        .onSuccess([this](const std::string &name, uint32_t, uint32_t) {
            m_sub_chan->bindQueue("game.state", name, "");
            m_sub_chan->consume(name)
                .onReceived([this](const AMQP::Message &msg, uint64_t tag, bool) {
                    pacman::GameState gs;
                    if (gs.ParseFromArray(msg.body(), (int)msg.bodySize())) {
                        std::lock_guard<std::mutex> lk(m_state_mutex);
                        m_state       = gs;
                        m_state_ready = true;
                    }
                    m_sub_chan->ack(tag);
                });
        });

    // Pump a bit so queue declare + bind + consume all complete
    for (int i = 0; i < 50; i++) {
        m_sub_handler->process(m_sub_conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ClientWidget::setupPublisher()
{
    m_pub_handler = new SimpleAMQPHandler(m_host, (uint16_t)m_port);
    m_pub_conn    = new AMQP::Connection(m_pub_handler,
                        AMQP::Login(m_user, m_password), "/");
    waitReady(m_pub_handler, m_pub_conn);

    m_pub_chan = new AMQP::Channel(m_pub_conn);
    m_pub_chan->declareQueue("input.keys", AMQP::durable);

    for (int i = 0; i < 20; i++) {
        m_pub_handler->process(m_pub_conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ClientWidget::subscriberLoop()
{
    while (m_running) {
        m_sub_handler->process(m_sub_conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void ClientWidget::publishKey(pacman::KeyAction action, bool pressed)
{
    pacman::KeyInput ki;
    ki.set_action(action);
    ki.set_pressed(pressed);

    std::string payload;
    ki.SerializeToString(&payload);

    AMQP::Envelope env(payload.data(), payload.size());
    env.setContentType("application/octet-stream");
    m_pub_chan->publish("", "input.keys", env);
    // Flush
    m_pub_handler->process(m_pub_conn);
}

// ── Key events ────────────────────────────────────────────────────────────────

void ClientWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;
    switch (event->key()) {
    case Qt::Key_Up:    publishKey(pacman::KEY_UP,    true); break;
    case Qt::Key_Down:  publishKey(pacman::KEY_DOWN,  true); break;
    case Qt::Key_Left:  publishKey(pacman::KEY_LEFT,  true); break;
    case Qt::Key_Right: publishKey(pacman::KEY_RIGHT, true); break;
    default: break;
    }
}

void ClientWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;
    switch (event->key()) {
    case Qt::Key_Up:    publishKey(pacman::KEY_UP,    false); break;
    case Qt::Key_Down:  publishKey(pacman::KEY_DOWN,  false); break;
    case Qt::Key_Left:  publishKey(pacman::KEY_LEFT,  false); break;
    case Qt::Key_Right: publishKey(pacman::KEY_RIGHT, false); break;
    default: break;
    }
}

// ── OpenGL ────────────────────────────────────────────────────────────────────

void ClientWidget::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    float diff[4] = {1,1,0,0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    float amb[4] = {1,0,1,1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

    createDisplayLists();
}

void ClientWidget::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / h, 0.005, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(-1.5, 0, 40, -1.5, 0, 0, 0, 1, 0);
}

void ClientWidget::onRedraw()
{
    update();
}

void ClientWidget::drawSphere(double r, int sl, int st)
{
    GLUquadric *q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, r, sl, st);
    gluDeleteQuadric(q);
}

void ClientWidget::createDisplayLists()
{
    m_list[1] = glGenLists(1);
    glNewList(m_list[1], GL_COMPILE);
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);
    glVertex3f(1,1,1); glVertex3f(1,1,0); glVertex3f(0,1,0); glVertex3f(0,1,1);
    glEnd();
    glEndList();

    m_list[2] = glGenLists(1);
    glNewList(m_list[2], GL_COMPILE);
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);
    glVertex3f(1,1,1); glVertex3f(1,1,0); glVertex3f(0,1,0); glVertex3f(0,1,1);
    glNormal3f(0,-1,0);
    glVertex3f(1,0,0); glVertex3f(1,0,1); glVertex3f(0,0,1); glVertex3f(0,0,0);
    glEnd();
    glEndList();

    m_list[3] = glGenLists(1);
    glNewList(m_list[3], GL_COMPILE);
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);
    glVertex3f(1,1,1); glVertex3f(1,1,0); glVertex3f(0,1,0); glVertex3f(0,1,1);
    glNormal3f(1,0,0);
    glVertex3f(1,1,0); glVertex3f(1,1,1); glVertex3f(1,0,1); glVertex3f(1,0,0);
    glEnd();
    glEndList();

    m_list[4] = glGenLists(1);
    glNewList(m_list[4], GL_COMPILE);
    glBegin(GL_QUADS);
    glColor3f(0,0.3f,0);
    glNormal3f(1,0,1);
    glVertex3f(1,1,1); glVertex3f(0,1,1); glVertex3f(0,0,1); glVertex3f(1,0,1);
    glEnd();
    glEndList();
}

// ── Scene rendering from received state ──────────────────────────────────────

void ClientWidget::drawScene(const pacman::GameState &gs)
{
    // Board (display lists, same as standalone client)
    for (int pass = 0; pass < 2; pass++) {
        int j_start = (pass == 0) ? 0 : BOARD_Y / 2;
        int j_end   = (pass == 0) ? BOARD_Y / 2 : BOARD_Y;
        int j_step  = (pass == 0) ? 1 : -1;
        int j_first = (pass == 0) ? j_start : j_end - 1;
        int j_last  = (pass == 0) ? j_end   : j_start - 1;

        // Hardcoded board layout (same as server) - reuse BOARD_INIT
        extern const int BOARD_INIT[BOARD_X][BOARD_Y];  // defined in PacmanServer.cpp via linker -- use a local copy
        // For the client, board geometry is static (walls never change)
        static const int board[BOARD_X][BOARD_Y] = {
            {8,5,5,5,5,5,5,5,5,5,5,5,5,1,1,5,5,5,5,5,5,5,5,5,5,5,5,7},
            {6,0,0,0,0,0,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,0,0,0,0,0,0,6},
            {6,0,8,1,1,7,0,8,1,1,1,7,0,2,4,0,8,1,1,1,7,0,8,1,1,7,0,6},
            {6,0,2,11,11,4,0,2,11,11,11,4,0,2,4,0,2,11,11,11,4,0,2,11,11,4,0,6},
            {6,0,9,3,3,10,0,9,3,3,3,10,0,9,10,0,9,3,3,3,10,0,9,3,3,10,0,6},
            {6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
            {6,0,8,1,1,7,0,8,7,0,8,1,1,1,1,1,1,7,0,8,7,0,8,1,1,7,0,6},
            {6,0,9,3,3,10,0,2,4,0,9,3,3,11,11,3,3,10,0,2,4,0,9,3,3,10,0,6},
            {6,0,0,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,0,0,6},
            {9,5,5,5,5,7,0,2,11,1,1,7,0,2,4,0,8,1,1,11,4,0,8,5,5,5,5,10},
            {0,0,0,0,0,6,0,2,11,3,3,10,0,9,10,0,9,3,3,11,4,0,6,0,0,0,0,0},
            {0,0,0,0,0,6,0,2,4,0,0,0,0,0,0,0,0,0,0,2,4,0,6,0,0,0,0,0},
            {0,0,0,0,0,6,0,2,4,0,8,5,5,1,1,5,5,7,0,2,4,0,6,0,0,0,0,0},
            {5,5,5,5,5,10,0,9,10,0,6,0,0,0,0,0,0,6,0,9,10,0,9,5,5,5,5,5},
            {0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0},
            {5,5,5,5,5,7,0,8,7,0,6,0,0,0,0,0,0,6,0,8,7,0,8,5,5,5,5,5},
            {0,0,0,0,0,6,0,2,4,0,9,5,5,5,5,5,5,10,0,2,4,0,6,0,0,0,0,0},
            {0,0,0,0,0,6,0,2,4,0,0,0,0,0,0,0,0,0,0,2,4,0,6,0,0,0,0,0},
            {0,0,0,0,0,6,0,2,4,0,8,1,1,1,1,1,1,7,0,2,4,0,6,0,0,0,0,0},
            {8,5,5,5,5,10,0,9,10,0,9,3,3,11,11,3,3,10,0,9,10,0,9,5,5,5,5,7},
            {6,0,0,0,0,0,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,0,0,0,0,0,0,6},
            {6,0,8,1,1,7,0,8,1,1,1,7,0,2,4,0,8,1,1,1,7,0,8,1,1,7,0,6},
            {6,0,9,3,11,4,0,9,3,3,3,10,0,9,10,0,9,3,3,3,10,0,2,11,3,10,0,6},
            {6,0,0,0,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,0,0,0,6},
            {2,1,7,0,2,4,0,8,7,0,8,1,1,1,1,1,1,7,0,8,7,0,2,4,0,8,1,4},
            {2,3,10,0,9,10,0,2,4,0,9,3,3,11,11,3,3,10,0,2,4,0,9,10,0,9,3,4},
            {6,0,0,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,2,4,0,0,0,0,0,0,6},
            {6,0,8,1,1,1,1,11,11,1,1,7,0,2,4,0,8,1,1,11,11,1,1,1,1,7,0,6},
            {6,0,9,3,3,3,3,3,3,3,3,10,0,9,10,0,9,3,3,3,3,3,3,3,3,10,0,6},
            {6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
            {9,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,10}
        };

        for (int i = 0; i < BOARD_X; i++) {
            for (int j = j_first; (pass==0)?(j<j_last):(j>=j_last); j+=j_step) {
                glColor3f(0,0,1);
                int call_this = 0;
                glPushMatrix();
                glTranslatef(-(float)BOARD_X/2,(-(float)BOARD_Y/2),0);
                glTranslatef((float)j,(float)(BOARD_Y-i),0);
                glPushMatrix();
                glTranslatef(0.5f,0.5f,0);
                switch (board[i][j]) {
                case 4: glRotatef(90,0,0,1); // fall
                case 3: glRotatef(90,0,0,1); // fall
                case 2: glRotatef(90,0,0,1); // fall
                case 1: call_this=1; break;
                case 6: glRotatef(90,0,0,1); // fall
                case 5: call_this=2; break;
                case 10: glRotatef(90,0,0,1); // fall
                case 9:  glRotatef(90,0,0,1); // fall
                case 8:  glRotatef(90,0,0,1); // fall
                case 7:  call_this=3; break;
                }
                glScalef(1,1,0.5f);
                glTranslatef(-0.5f,-0.5f,0);
                glCallList(m_list[call_this]);
                glPopMatrix();
                if (call_this!=0 || board[i][j]==11) {
                    glTranslatef(0,0,-0.5f);
                    glCallList(m_list[4]);
                }
                glPopMatrix();

                // Pebbles (sent in state)
                int idx = i * BOARD_Y + j;
                int pv  = (idx < gs.pebble_array_size()) ? gs.pebble_array(idx) : 0;
                if (pv > 0) {
                    glColor3f(0,1.0f,1.0f/(float)pv);
                    glPushMatrix();
                    glTranslatef(-(float)BOARD_X/2,-(float)BOARD_Y/2,0);
                    glTranslatef((float)j,(float)(BOARD_Y-i),0);
                    glTranslatef(0.5f,0.5f,0.5f);
                    drawSphere(0.1f*(float)pv,6,6);
                    glPopMatrix();
                }
            }
        }
    }

    // Pacman
    glColor3f(0,1,1);
    glPushMatrix();
    glTranslatef((float)gs.pac_x(),-(float)gs.pac_y(),0);
    glTranslatef(0.5f,0.6f,0);
    glTranslatef((float)BOARD_X/-2.0f,(float)BOARD_Y/2.0f,0.5f);
    drawSphere(0.5,15,10);
    glPopMatrix();

    // Ghosts
    for (int d = 0; d < gs.ghosts_size(); d++) {
        const auto &g = gs.ghosts(d);
        if (!g.edible())
            glColor3f(g.color_r(), g.color_g(), g.color_b());
        else {
            int et = g.edible_timer();
            if (et < 150) glColor3f((et/10)%2,(et/10)%2,1);
            else          glColor3f(0,0,1);
        }
        if (g.eaten()) glColor3f(1,1,0);

        glPushMatrix();
        glTranslatef((float)g.x(),-(float)g.y(),0);
        glTranslatef(0.5f,0.6f,0);
        glTranslatef((float)BOARD_X/-2.0f,(float)BOARD_Y/2.0f,0.5f);
        drawSphere(0.5,10,10);
        glPopMatrix();
    }
}

// ── paintGL ───────────────────────────────────────────────────────────────────

void ClientWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pacman::GameState gs;
    bool ready = false;
    {
        std::lock_guard<std::mutex> lk(m_state_mutex);
        if (m_state_ready) { gs = m_state; ready = true; }
    }

    if (ready) drawScene(gs);

    // HUD
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font("Arial", 14, QFont::Bold);
    painter.setFont(font);
    int ww = width(), wh = height();

    if (!ready) {
        painter.setPen(QColor(255,255,255));
        painter.drawText(QRect(0,0,ww,wh), Qt::AlignCenter, "Connecting to server...");
        painter.end();
        return;
    }

    painter.setPen(QColor(255,0,0));
    painter.drawText(10, 24, "PAC MAN");

    painter.setPen(QColor(255,255,0));
    painter.drawText(ww/2-50, 24, QString("Points: %1").arg(gs.points()));
    painter.drawText(ww-120,  24, QString("Lives: %1").arg(gs.lives()));

    if (gs.gameover()) {
        QFont big("Arial", 28, QFont::Bold);
        painter.setFont(big);
        painter.setPen(QColor(255,0,0));
        painter.drawText(QRect(0,wh/2-20,ww,40), Qt::AlignCenter, "GAME OVER");
    } else if (gs.start_wait()) {
        QFont big("Arial", 22, QFont::Bold);
        painter.setFont(big);
        painter.setPen(QColor(255,255,255));
        painter.drawText(QRect(0,wh/2-30,ww,50), Qt::AlignCenter, "Press arrow key to start!");
    }

    painter.end();
}
