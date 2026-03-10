#include <iostream>
#include "my_logger/logger.h"
#include <thread>
#include <chrono>
#include "thread_pool/epool.h"
#include "my_net/my_socket.h"
#include "config.h"
#include "init/server.h"
#include <sys/epoll.h>
#include <signal.h>

// Global pointer so the signal handler can notify the server to stop.
// This avoids capturing local variables inside a C-style signal handler.
static Server* g_server = nullptr;

static void handle_signal(int /*signum*/) {
    if (g_server) {
        g_server->stop();
    }
}

int main() {
    // Prevent SIGPIPE from terminating the process when writing to closed sockets.
    signal(SIGPIPE, SIG_IGN);

    // Create server and expose it to the signal handler.
    Server server(Config::SERVER_PORT, Config::LOG_PATH, Config::THREAD_POOL_SIZE);
    g_server = &server;

    // Register SIGINT / SIGTERM to allow graceful shutdown
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    server.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
