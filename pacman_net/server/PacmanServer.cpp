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

#include "PacmanServer.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <chrono>
#include <thread>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ── SimpleAMQPHandler implementation ─────────────────────────────────────────

SimpleAMQPHandler::SimpleAMQPHandler(const std::string &host, uint16_t port)
    : m_recvBuf(65536)
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    struct addrinfo hints{};
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *result = nullptr;
    std::string portStr = std::to_string(port);
    int ret = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
    if (ret != 0 || result == nullptr)
        throw std::runtime_error("Cannot resolve host: " + host);

    m_sock = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);

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
        int n = ::send(m_sock, data + sent, (int)(size - sent), 0);
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
    if (n > 0) conn->parse(m_recvBuf.data(), n);
}

static void waitReady(SimpleAMQPHandler *h, AMQP::Connection *c)
{
    for (int i = 0; i < 200 && !h->ready(); i++) {
        h->process(c);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!h->ready()) throw std::runtime_error("AMQP handshake timed out");
}

// ── setupPublisher / setupSubscriber ─────────────────────────────────────────

void PacmanServer::setupPublisher()
{
    m_pub_handler = new SimpleAMQPHandler(m_host, (uint16_t)m_port);
    m_pub_conn    = new AMQP::Connection(m_pub_handler,
                        AMQP::Login(m_user, m_password), "/");
    waitReady(m_pub_handler, m_pub_conn);

    m_pub_chan = new AMQP::Channel(m_pub_conn);
    m_pub_chan->declareExchange("game.state", AMQP::fanout, AMQP::durable);
    m_pub_chan->declareQueue("input.keys", AMQP::durable);

    for (int i = 0; i < 20; i++) {
        m_pub_handler->process(m_pub_conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void PacmanServer::setupSubscriber()
{
    m_sub_handler = new SimpleAMQPHandler(m_host, (uint16_t)m_port);
    m_sub_conn    = new AMQP::Connection(m_sub_handler,
                        AMQP::Login(m_user, m_password), "/");
    waitReady(m_sub_handler, m_sub_conn);

    m_sub_chan = new AMQP::Channel(m_sub_conn);
    m_sub_chan->consume("input.keys")
        .onReceived([this](const AMQP::Message &msg, uint64_t tag, bool) {
            pacman::KeyInput ki;
            if (ki.ParseFromArray(msg.body(), (int)msg.bodySize())) {
                m_key_action  = (int)ki.action();
                m_key_pressed = ki.pressed();
            }
            m_sub_chan->ack(tag);
        });

    for (int i = 0; i < 20; i++) {
        m_sub_handler->process(m_sub_conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void PacmanServer::subscriberLoop()
{
    while (m_running) {
        m_sub_handler->process(m_sub_conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// ── Board data ────────────────────────────────────────────────────────────────

static const int BOARD_INIT[BOARD_X][BOARD_Y] = {
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

static const int PEBBLE_INIT[BOARD_X][BOARD_Y] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,3,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,3,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0},
    {0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0},
    {0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,3,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,3,0},
    {0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0},
    {0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0},
    {0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0},
    {0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0},
    {0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// ── AMQP-CPP uses an event-loop handler. We use a simple blocking TCP handler.
// On Windows, use the Winsock handler; on Linux, use the linux_tcp handler.
// AMQP-CPP's TcpConnection drives itself when you call connection.process().
// We run it in a tight loop on the subscriber thread.

PacmanServer::PacmanServer(const std::string &host, int port,
                           const std::string &user, const std::string &password)
    : m_host(host), m_port(port), m_user(user), m_password(password)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    initBoard();
    resetPebbles();
    reinitPlayer();

    static const int   start_x[4]        = {11, 12, 15, 16};
    static const float ghost_colors[4][3] = {
        {1,0,0},{0.47f,0.94f,0.47f},{1,0.78f,0.78f},{1,0.49f,0}
    };
    for (int i = 0; i < 4; i++) {
        ghost[i] = new Ghost(start_x[i], 14);
        ghost[i]->max_speed = 0.1f - 0.01f * i;
        ghost[i]->speed     = ghost[i]->max_speed;
        for (int j = 0; j < 3; j++)
            ghost[i]->color[j] = ghost_colors[i][j];
    }
    num_ghosts  = 4;
    start_timer = 3;
    gameover    = false;
    animate     = false;

    setupPublisher();
    setupSubscriber();
}

PacmanServer::~PacmanServer()
{
    delete m_pub_chan; delete m_pub_conn; delete m_pub_handler;
    delete m_sub_chan; delete m_sub_conn; delete m_sub_handler;
    for (int i = 0; i < num_ghosts; i++) delete ghost[i];
    google::protobuf::ShutdownProtobufLibrary();
}

// ── AMQP-CPP setup ────────────────────────────────────────────────────────────


void PacmanServer::publishState()
{
    pacman::GameState gs;
    {
        std::lock_guard<std::mutex> lk(m_state_mutex);
        gs.set_pac_x(pac_x);
        gs.set_pac_y(pac_y);
        gs.set_pac_angle(pac_angle);
        gs.set_animate(animate);
        gs.set_lives(lives);
        gs.set_points(points);
        gs.set_gameover(gameover);
        gs.set_start_wait(start_timer > 0);
        gs.set_pebbles_left(pebbles_left);

        for (int i = 0; i < num_ghosts; i++) {
            auto *g = gs.add_ghosts();
            g->set_x(ghost[i]->x);
            g->set_y(ghost[i]->y);
            g->set_angle(ghost[i]->angle);
            g->set_edible(ghost[i]->edible);
            g->set_eaten(ghost[i]->eaten);
            g->set_color_r(ghost[i]->color[0]);
            g->set_color_g(ghost[i]->color[1]);
            g->set_color_b(ghost[i]->color[2]);
            g->set_edible_timer(ghost[i]->edible_timer);
        }
        for (int i = 0; i < BOARD_X; i++)
            for (int j = 0; j < BOARD_Y; j++)
                gs.add_pebble_array(tp_array[i][j]);
    }

    std::string payload;
    gs.SerializeToString(&payload);

    AMQP::Envelope env(payload.data(), payload.size());
    env.setContentType("application/octet-stream");
    env.setDeliveryMode(1);
    m_pub_chan->publish("game.state", "", env);
    m_pub_handler->process(m_pub_conn);  // flush
}

void PacmanServer::run()
{
    m_running = true;
    std::thread subThread(&PacmanServer::subscriberLoop, this);

    auto tick = std::chrono::milliseconds(16);
    while (m_running) {
        auto t0 = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lk(m_state_mutex);
            if (!gameover) { applyKey(); if (animate) movePac(); updateGhosts(); }
        }
        publishState();

        auto rem = tick - (std::chrono::steady_clock::now() - t0);
        if (rem > std::chrono::milliseconds(0))
            std::this_thread::sleep_for(rem);
    }
    subThread.join();
}

void PacmanServer::stop() { m_running = false; }

void PacmanServer::initBoard()   { memcpy(board_array,  BOARD_INIT,  sizeof(board_array)); }
void PacmanServer::resetPebbles(){ memcpy(tp_array, PEBBLE_INIT, sizeof(tp_array));
                                   memcpy(pebble_array, PEBBLE_INIT, sizeof(pebble_array));
                                   pebbles_left = 244; }

void PacmanServer::reinitPlayer()
{
    pac_x = 13.5; pac_y = 23; pac_angle = 90; pac_speed = 0.1; animate = false;
}

void PacmanServer::reinitGhosts()
{
    static const int start_x[4] = {11,12,15,16};
    for (int i = 0; i < num_ghosts; i++) {
        ghost[i]->Reinit();
        ghost[i]->x = start_x[i]; ghost[i]->y = 14;
        ghost[i]->eaten = false;
        ghost[i]->jail_timer = i*33+66;
        ghost[i]->max_speed = 0.1f-0.01f*i;
        ghost[i]->speed = ghost[i]->max_speed;
    }
    start_timer = 3;
}

bool PacmanServer::cellOpen(int col, int row)
{
    if (col<0||col>=BOARD_Y||row<0||row>=BOARD_X) return false;
    return board_array[row][col]==0;
}

void PacmanServer::applyKey()
{
    int  action  = m_key_action.load();
    bool pressed = m_key_pressed.load();
    if (!pressed || start_timer>0) return;
    int nx=(int)pac_x, ny=(int)pac_y;
    switch(action){
    case pacman::KEY_UP:
        if((int)pac_x-pac_x>-0.1&&(int)pac_angle!=270)
            if(cellOpen(nx,ny-1)){animate=true;pac_angle=270;} break;
    case pacman::KEY_DOWN:
        if((int)pac_x-pac_x>-0.1&&(int)pac_angle!=90)
            if(cellOpen(nx,ny+1)){animate=true;pac_angle=90;} break;
    case pacman::KEY_LEFT:
        if((int)pac_y-pac_y>-0.1&&(int)pac_angle!=180)
            if(cellOpen(nx-1,ny)){animate=true;pac_angle=180;} break;
    case pacman::KEY_RIGHT:
        if((int)pac_y-pac_y>-0.1&&(int)pac_angle!=0)
            if(cellOpen(nx+1,ny)){animate=true;pac_angle=0;} break;
    default: break;
    }
}

void PacmanServer::movePac()
{
    pac_x += pac_speed*cos(M_PI/180.0*pac_angle);
    pac_y += pac_speed*sin(M_PI/180.0*pac_angle);

    if((int)pac_x==27&&(int)pac_y==14&&(int)pac_angle==0) pac_x=0;
    else if((int)(pac_x+0.9)==0&&(int)pac_y==14&&(int)pac_angle==180) pac_x=27;

    if(!cellOpen((int)(pac_x+cos(M_PI/180.0*pac_angle)),
                 (int)(pac_y+sin(M_PI/180.0*pac_angle)))&&
       pac_x-(int)pac_x<0.1&&pac_y-(int)pac_y<0.1)
        animate=false;

    int pi=(int)(pac_y+0.5), pj=(int)(pac_x+0.5);
    if(tp_array[pi][pj]==1){ tp_array[pi][pj]=0; pebbles_left--; points++; }
    else if(tp_array[pi][pj]==3){
        tp_array[pi][pj]=0; pebbles_left--; points+=5;
        for(int i=0;i<num_ghosts;i++) if(!ghost[i]->eaten) ghost[i]->Vulnerable();
    }
    if(pebbles_left==0){ reinitGhosts(); reinitPlayer(); resetPebbles(); lives=3; points=0; }
}

void PacmanServer::updateGhosts()
{
    for(int d=0;d<num_ghosts;d++){
        if(start_timer==0) ghost[d]->Update();

        if(!ghost[d]->in_jail&&
           ghost[d]->x-(int)ghost[d]->x<0.1&&
           ghost[d]->y-(int)ghost[d]->y<0.1){
            bool om[4];
            for(int a=0;a<4;a++)
                om[a]=cellOpen((int)(ghost[d]->x+cos(M_PI/180.0*a*90)),
                               (int)(ghost[d]->y+sin(M_PI/180.0*a*90)));
            ghost[d]->Chase(!ghost[d]->eaten?pac_x:13,
                            !ghost[d]->eaten?pac_y:11,om);
        }

        if(ghost[d]->in_jail&&
           !cellOpen((int)(ghost[d]->x+cos(M_PI/180.0*ghost[d]->angle)),
                     (int)(ghost[d]->y+sin(M_PI/180.0*ghost[d]->angle)))&&
           ghost[d]->jail_timer>0&&
           ghost[d]->x-(int)ghost[d]->x<0.1&&
           ghost[d]->y-(int)ghost[d]->y<0.1)
            ghost[d]->angle=(double)(((int)ghost[d]->angle+180)%360);

        if(start_timer==0) ghost[d]->Move();

        if(!ghost[d]->eaten&&ghost[d]->Catch(pac_x,pac_y)){
            if(!ghost[d]->edible){
                if(lives>0) lives--;
                if(lives==0) gameover=true;
                reinitPlayer();
            } else {
                ghost[d]->edible=false; ghost[d]->eaten=true; ghost[d]->speed=1;
            }
        }
    }
    if(start_timer>0) start_timer--;
}
