#ifndef SOCKET_H
#define SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
class MySocket {
private:
    int sockfd;
    int port;
    sockaddr_in addr;

public:
    MySocket(int port):port(port) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        
    }
    ~MySocket() {
        close(sockfd);
    }
    void bindSocket() {
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("Bind failed");
        }

    }
    void listenSocket(int backlog = 5) {
        if (listen(sockfd, backlog) < 0) {
            throw std::runtime_error("Listen failed");
        }
        
    }
    int acceptConnection() {
        int clientfd = accept(sockfd, nullptr, nullptr);
        if (clientfd < 0) {
            throw std::runtime_error("Accept failed");
        }
        return clientfd;
    }
    int getSocketFD() const {
        return sockfd;
    }
};
#endif // SOCKET_H