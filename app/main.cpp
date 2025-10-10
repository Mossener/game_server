#include <iostream>
#include "logger.h"
#include <thread>
#include <chrono>
#include "epool.h"
#include "my_socket.h"
#include <sys/epoll.h>
int main() {
    Epool pool(4); 
    Logger logger(std::string(LOGGER_PATH) +"/app.log");
    try {
        MySocket serverSocket(8080);
        serverSocket.bindSocket();
        serverSocket.listenSocket();
        logger.addToQueue("Server started, listening on port 8080", LogLevel::INFO);
        int epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            throw std::runtime_error("Failed to create epoll file descriptor");
        }
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = serverSocket.getSocketFD();
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket.getSocketFD(), &event) == -1) {
            throw std::runtime_error("Failed to add file descriptor to epoll");
        }
        const int MAX_EVENTS = 10;
        struct epoll_event events[MAX_EVENTS];
        while (true) {
            int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (event_count == -1) {
                throw std::runtime_error("epoll_wait failed");
            }
            for (int i = 0; i < event_count; i++) {
                if (events[i].data.fd == serverSocket.getSocketFD()) {
                    int client_fd = serverSocket.acceptConnection();
                    logger.addToQueue("Accepted new connection", LogLevel::INFO);
                    pool.enqueue([client_fd, &logger]() {
                        char buffer[1024];
                        ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer) - 1);
                        if (bytesRead > 0) {
                            buffer[bytesRead] = '\0';
                            logger.addToQueue(std::string("Received: ") + buffer, LogLevel::INFO);
                            const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
                            write(client_fd, response, strlen(response));
                        } else {
                            logger.addToQueue("Read error or connection closed by client", LogLevel::WARNING);
                        }
                        close(client_fd);
                        logger.addToQueue("Closed connection", LogLevel::INFO);
                    });
                }
            }
        }
    }catch (const std::exception &e) {
        logger.addToQueue(std::string("Server error: ") + e.what(), LogLevel::ERROR);
        return 1;
    }
    // Give some time for the logging thread to process messages
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}