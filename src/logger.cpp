#include "logger.h"
#include <iostream>
#include <fstream>
void Logger::log(const std::string &message, const LogLevel &level) {
    std::cerr << "Log path: " << logFilePath << std::endl;

    std::ofstream logFile(logFilePath, std::ios_base::app);
    if (logFile.is_open()) {
        logFile << "[" << logLevels[static_cast<int>(level)] << "] " << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Unable to open log file: " << logFilePath << std::endl;
    }
}