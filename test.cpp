#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
int main() {
    const char * message ;
    scanf("%s", message);
    std::cout << message << std::endl;
    send(1, message, strlen(message), 0);
    return 0;
}