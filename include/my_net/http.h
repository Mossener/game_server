#ifndef HTTP_H
#define HTTP_H
#include <string>
#include <map>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include "my_net/my_socket.h"

struct HttpRequest{
    std::string version;
    std::string function;
    std::string path;
    std::map<std::string,std::string> head_body;
    std::string body;
};

// 修正结构体拼写
struct HttpResponse
{
    std::string version;
    int status_code;
    std::string status_msg;
    std::map<std::string,std::string> head_body;
    std::string body;
};

class HttpHandle{
public:
    void HttpHandle_f(int socket_fd){
        HttpRequest req;
        HttpResponse res;  // 同步修正结构体名

        // 读取头部（处理EINTR）
        std::string buf;
        char temp[1024];
        ssize_t n;
        while (true) {
            n = read(socket_fd, temp, sizeof(temp));
            if (n == -1) {
                if (errno == EINTR) continue;
                throw std::runtime_error("read header error: " + std::string(strerror(errno)));
            } else if (n == 0) {
                throw std::runtime_error("client closed prematurely");
            }
            buf.append(temp, n);
            if (buf.find("\r\n\r\n") != std::string::npos) break;
        }

        // 解析头部和body
        size_t header_end = buf.find("\r\n\r\n");
        std::string header_str = buf.substr(0, header_end);
        std::string body_str = buf.substr(header_end + 4);  // 跳过\r\n\r\n

        std::istringstream iss_header(header_str);
        iss_header >> req.function >> req.path >> req.version;
        // 移除version中的\r
        if (!req.version.empty() && req.version.back() == '\r') {
            req.version.pop_back();
        }

        std::string head_line;
        size_t content_length = 0;
        while (getline(iss_header, head_line)) {
            if (!head_line.empty() && head_line.back() == '\r') {
                head_line.pop_back();
            }
            if (head_line.empty()) break;
            size_t index_pos = head_line.find(':');
            if (index_pos == std::string::npos) continue;
            std::string key = head_line.substr(0, index_pos);
            std::string value = head_line.substr(index_pos + 2);
            req.head_body[key] = value;

            // 大小写不敏感判断Content-Length
            if (to_lower(key) == "content-length") {
                try {
                    content_length = std::stoul(value);
                } catch (...) {
                    throw std::runtime_error("invalid Content-Length");
                }
            }
        }

        // 补充读取body（处理EINTR）
        while (body_str.size() < content_length) {
            n = read(socket_fd, temp, sizeof(temp));
            if (n == -1) {
                if (errno == EINTR) continue;
                throw std::runtime_error("read body error: " + std::string(strerror(errno)));
            } else if (n == 0) {
                throw std::runtime_error("body incomplete");
            }
            body_str.append(temp, n);
        }
        req.body = body_str;

        // 构建响应
        res.version = "HTTP/1.1";
        if (req.path == "/" || req.path == "/index.html") {
            res.status_code = 200;
            res.status_msg = "OK";
            res.head_body["Content-Type"] = "text/html";
            res.body = "<html><body><h1>Hello, HTTP Server!</h1></body></html>";
        } else if (req.path == "/about") {
            res.status_code = 200;
            res.status_msg = "OK";
            res.head_body["Content-Type"] = "text/plain";
            res.body = "This is a simple HTTP server.";
        } else {
            res.status_code = 404;
            res.status_msg = "Not Found";
            res.head_body["Content-Type"] = "text/plain";
            res.body = "Page not found: " + req.path;
        }

        res.head_body["Content-Length"] = std::to_string(res.body.size());
        res.head_body["Connection"] = "close";

        // 构建响应字符串
        std::string response = res.version + " " + std::to_string(res.status_code) + " " + res.status_msg + "\r\n";
        for (auto&[key, value] : res.head_body) {
            response += key + ": " + value + "\r\n";
        }
        response += "\r\n" + res.body;

        // 完整发送响应
        const char* data = response.c_str();
        size_t total = response.size();
        size_t sent = 0;
        while (sent < total) {
            ssize_t b_send = send(socket_fd, data + sent, total - sent, 0);
            if (b_send == -1) {
                if (errno == EINTR) continue;
                throw std::runtime_error("send error: " + std::string(strerror(errno)));
            }
            sent += b_send;
        }

 // 关闭连接
        close(socket_fd);
    }

private:
    // 辅助函数：字符串转小写
    std::string to_lower(const std::string& s) {
        std::string res = s;
        std::transform(res.begin(), res.end(), res.begin(), ::tolower);
        return res;
    }
};
#endif
