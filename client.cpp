#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Socket creation failed");
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    //这是一个连接服务器的客户端套接字
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Connect failed");
    }
    const char *message = "Hello, Server!";
    send(sockfd, message, strlen(message), 0);
    char buffer[1024];
    ssize_t bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Received from server: " << buffer << std::endl;
    } else {
        std::cerr << "Read error or connection closed by server" << std::endl;
    }
    close(sockfd);
    
    return 0;
}