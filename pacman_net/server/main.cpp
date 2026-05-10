#include <iostream>
#include <csignal>
#include "PacmanServer.h"

static PacmanServer *g_server = nullptr;

static void handleSignal(int)
{
    if (g_server) g_server->stop();
}

int main(int argc, char **argv)
{
    std::string host     = (argc > 1) ? argv[1] : "localhost";
    int         port     = (argc > 2) ? std::stoi(argv[2]) : 5672;
    std::string user     = (argc > 3) ? argv[3] : "guest";
    std::string password = (argc > 4) ? argv[4] : "guest";

    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    try {
        PacmanServer server(host, port, user, password);
        g_server = &server;
        std::cout << "Pacman server running. Ctrl+C to stop.\n";
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
