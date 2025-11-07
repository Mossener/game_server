#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <queue>
#include <condition_variable>
#include "file_io/rall.h"
#include <thread>
enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};
class Logger {
private:
    const std::string logFilePath;
    const std::vector<std::string> logLevels = {"INFO", "WARNING", "ERROR"};    
    std::mutex logMutex;
    std::queue<std::pair<std::string, LogLevel>> logQueue;
    //条件变量
    std::condition_variable logCondition;
    std::thread worker;
    bool running = true;
public:
    Logger(const std::string &filePath) : logFilePath(filePath) {
        worker = std::thread(&Logger::log, this);
    }
    ~Logger() {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            running = false;
        }
        logCondition.notify_all();
        if (worker.joinable()) {
            worker.join();
        }
    }
    void log();
    void addToQueue(const std::string &message, const LogLevel &level);

};
#endif // LOGGER_H