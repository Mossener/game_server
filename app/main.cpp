#include <iostream>
#include "logger.h"
#include <thread>
#include <chrono>

int main() {
    std::string logFilePath = std::string(LOGGER_PATH) +"/app.log";
    Logger logger(logFilePath);
    logger.log("Application started", LogLevel::INFO);
    logger.log("Potential issue detected", LogLevel::WARNING);
    logger.log("Critical error occurred", LogLevel::ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}