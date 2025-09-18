#include <iostream>
#include "logger.h"
#include <thread>
#include <chrono>

int main() {
    std::string logFilePath = std::string(LOGGER_PATH) +"/app.log";
    Logger logger(logFilePath);


    logger.addToQueue("This is an info message.", LogLevel::INFO);
    logger.addToQueue("This is a warning message.", LogLevel::WARNING);
    logger.addToQueue("This is an error message.", LogLevel::ERROR);
    // Give some time for the logging thread to process messages
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}