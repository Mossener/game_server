#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <queue>
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
    const std::queue<std::pair<std::string, LogLevel>> logQueue;
public:
    Logger(const std::string &filePath) : logFilePath(filePath) {}
    void log(const std::string &message, const LogLevel &level);
};
#endif // LOGGER_H