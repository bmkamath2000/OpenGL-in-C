#pragma once

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

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <amqpcpp.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include "Constants.h"
#include "pacman.pb.h"

// ── Minimal blocking TCP handler for AMQP-CPP (no Boost/libuv needed) ─────────
// AMQP-CPP requires a ConnectionHandler; this one uses a plain blocking socket.
class SimpleAMQPHandler : public AMQP::ConnectionHandler
{
public:
    explicit SimpleAMQPHandler(const std::string &host, uint16_t port);
    ~SimpleAMQPHandler();

    // Called by AMQP-CPP to send bytes to the broker
    void onData(AMQP::Connection *conn,
                const char *data, size_t size) override;

    // Called by AMQP-CPP when the connection is ready
    void onReady(AMQP::Connection *) override { m_ready = true; }

    // Called on error
    void onError(AMQP::Connection *, const char *msg) override;

    // Called when connection is closed
    void onClosed(AMQP::Connection *) override { m_closed = true; }

    // Pump: read available bytes from socket and feed them to AMQP-CPP.
    // Call this periodically from your thread.
    void process(AMQP::Connection *conn);

    bool ready()  const { return m_ready;  }
    bool closed() const { return m_closed; }

private:
#ifdef _WIN32
    SOCKET m_sock = INVALID_SOCKET;
#else
    int    m_sock = -1;
#endif
    bool   m_ready  = false;
    bool   m_closed = false;

    std::vector<char> m_recvBuf;
};

// ── Qt client widget ──────────────────────────────────────────────────────────

class ClientWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(const std::string &host, int port,
                          const std::string &user, const std::string &password,
                          QWidget *parent = nullptr);
    ~ClientWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void onRedraw();

private:
    std::string m_host;
    int         m_port;
    std::string m_user;
    std::string m_password;

    // Subscriber (game state) — lives on background thread
    SimpleAMQPHandler  *m_sub_handler = nullptr;
    AMQP::Connection   *m_sub_conn    = nullptr;
    AMQP::Channel      *m_sub_chan    = nullptr;

    // Publisher (key input) — lives on main thread
    SimpleAMQPHandler  *m_pub_handler = nullptr;
    AMQP::Connection   *m_pub_conn    = nullptr;
    AMQP::Channel      *m_pub_chan    = nullptr;

    void setupPublisher();
    void setupSubscriber();
    void subscriberLoop();
    void publishKey(pacman::KeyAction action, bool pressed);

    std::mutex        m_state_mutex;
    pacman::GameState m_state;
    bool              m_state_ready = false;

    std::thread       m_receiver;
    std::atomic<bool> m_running{false};

    GLuint m_list[5];
    void   createDisplayLists();
    void   drawSphere(double r, int sl, int st);
    void   drawScene(const pacman::GameState &gs);

    QTimer *m_redrawTimer;
};
