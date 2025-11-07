#include "init/server.h"
#include <sys/epoll.h>
#include <stdexcept>
#include "config.h"
#include "time.h"
#include "my_net/http.h"
#include <unordered_set>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>
Server::Server(int port, const std::string &logFilePath, size_t threadPoolSize)
    : pool(threadPoolSize), serverSocket(port) {
    spdlog::init_thread_pool(8192, 1);
    logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_logger", logFilePath);
    logger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(logger);
    serverSocket.bindSocket();
    serverSocket.listenSocket();
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket.getSocketFD();
    logger->info("Server started on port {}", port);   
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket.getSocketFD(), &event) == -1) {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }
} 
Server::~Server() {
    spdlog::drop_all();
    spdlog::shutdown();
    close(epoll_fd);
}
void Server::start() {
    const int MAX_EVENTS = Config::MAX_EVENTS;
    struct epoll_event events[MAX_EVENTS];
    spdlog::info("Server started");
    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            if(errno == EINTR) continue;
            throw std::runtime_error("epoll_wait failed");
        }
        for(int i = 0; i < event_count; i++){
            int fd = events[i].data.fd;
            if(fd == serverSocket.getSocketFD()){
                int client_fd = serverSocket.acceptConnection();
                spdlog::info("New client with fd {} connected", client_fd);
                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    close(client_fd);
                    throw std::runtime_error("Failed to add file descriptor to epoll");
                }
                clientFds.insert(client_fd);
                printfd();
            }
            else{
                pool.enqueue(&HttpHandle::HttpHandle_f,&httph, fd);

            }
        }
    }
    return ;
}
void Server::handleClient(int client_fd) {
    char buffer[1024];
    ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        if(buffer[0] == 'q' && buffer[1] == 'u' && buffer[2] == 'i' && buffer[3] == 't'){
            spdlog::info("Client with fd {} requested to quit", client_fd);
            close(client_fd);
            clientFds.erase(client_fd);
            printfd();
            return;
        }
        spdlog::info("Received message from client with fd {}: {}", client_fd, buffer);
        //回写得到的数据
        const char *response = buffer;
        write(client_fd, response, strlen(response));
    } else {    
        spdlog::info("Client with fd {} disconnected", client_fd);
        close(client_fd);
        clientFds.erase(client_fd);
        printfd();
    }
}
void Server::printfd(){
    system("clear");
    std::cout<<"Current connected client fds:"<<std::endl;
    for(const auto &fd:clientFds){
        std::cout<<fd<<std::endl;
    }
    
}