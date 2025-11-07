#ifndef SERVER
#define SERVER
#include "thread_pool/epool.h"
#include "my_logger/logger.h"
#include <spdlog/spdlog.h>
#include "my_net/my_socket.h"
#include "my_net/http.h"
#include <unordered_set>
#include <memory>
class Server{
private:
    Epool pool; 
    std::shared_ptr<spdlog::logger> logger;
    MySocket serverSocket;
    int epoll_fd;
    std::unordered_set<int> clientFds;
    HttpHandle httph;
public:
    Server(int port, const std::string &logFilePath, size_t threadPoolSize);
    ~Server();
    void start();
    void handleClient(int clientFD);
    void printfd();
};


#endif