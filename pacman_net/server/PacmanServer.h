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

#include <amqpcpp.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <iostream>
#include "Ghost.h"
#include "Constants.h"
#include "pacman.pb.h"

// ── Minimal blocking TCP handler (same as client, shared by copy) ─────────────
class SimpleAMQPHandler : public AMQP::ConnectionHandler
{
public:
    explicit SimpleAMQPHandler(const std::string &host, uint16_t port);
    ~SimpleAMQPHandler();

    void onData(AMQP::Connection *, const char *data, size_t size) override;
    void onReady(AMQP::Connection *) override { m_ready = true; }
    void onError(AMQP::Connection *, const char *msg) override;
    void onClosed(AMQP::Connection *) override { m_closed = true; }

    void process(AMQP::Connection *conn);

    bool ready()  const { return m_ready;  }
    bool closed() const { return m_closed; }

private:
#ifdef _WIN32
    SOCKET m_sock = INVALID_SOCKET;
#else
    int    m_sock = -1;
#endif
    bool m_ready  = false;
    bool m_closed = false;
    std::vector<char> m_recvBuf;
};

// ── PacmanServer ──────────────────────────────────────────────────────────────
class PacmanServer
{
public:
    PacmanServer(const std::string &host, int port,
                 const std::string &user, const std::string &password);
    ~PacmanServer();

    void run();
    void stop();

private:
    std::string m_host;
    int         m_port;
    std::string m_user;
    std::string m_password;

    SimpleAMQPHandler  *m_pub_handler = nullptr;
    AMQP::Connection   *m_pub_conn    = nullptr;
    AMQP::Channel      *m_pub_chan    = nullptr;

    SimpleAMQPHandler  *m_sub_handler = nullptr;
    AMQP::Connection   *m_sub_conn    = nullptr;
    AMQP::Channel      *m_sub_chan    = nullptr;

    void setupPublisher();
    void setupSubscriber();
    void publishState();
    void subscriberLoop();

    int    board_array[BOARD_X][BOARD_Y];
    int    pebble_array[BOARD_X][BOARD_Y];
    int    tp_array[BOARD_X][BOARD_Y];

    int    pebbles_left;
    double pac_x, pac_y;
    double pac_angle;
    double pac_speed;
    bool   animate;
    int    lives;
    int    points;
    bool   gameover;
    int    start_timer;
    int    num_ghosts;

    Ghost *ghost[4];

    std::atomic<bool> m_running{false};
    std::mutex        m_state_mutex;
    std::atomic<int>  m_key_action{0};
    std::atomic<bool> m_key_pressed{false};

    void initBoard();
    void resetPebbles();
    void reinitPlayer();
    void reinitGhosts();
    bool cellOpen(int col, int row);
    void movePac();
    void updateGhosts();
    void applyKey();
};
