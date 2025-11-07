#include <iostream>
#include "my_logger/logger.h"
#include <thread>
#include <chrono>
#include "thread_pool/epool.h"
#include "my_net/my_socket.h"
#include "config.h"
#include "init/server.h"
#include <sys/epoll.h>
int main() {
    Server server(Config::SERVER_PORT, Config::LOG_PATH, Config::THREAD_POOL_SIZE);
    server.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}