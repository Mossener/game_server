#include "logger.h"
#include <iostream>
#include <fstream>
void Logger::log() {
    while(true){
        if(logQueue.empty()&&running == false){
            break;
        }
        std::unique_lock<std::mutex> lock(logMutex);
        logCondition.wait(lock, [this]{ return !logQueue.empty()||running == false; });
        if(logQueue.empty()&&running == false){
            break;
        }
        try{
            FILE_IO fileIO(logFilePath);
            std::ofstream &logFile = fileIO.getFile();
            if (logFile.is_open()) {
                logFile << "[" << logLevels[static_cast<int>(logQueue.front().second)] << "] " << logQueue.front().first << std::endl;
            } else {
            std::cerr << "Unable to open log file: " << logFilePath << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "Error logging message: " << e.what() << std::endl;
        }    
        logQueue.pop();
    }
}
void Logger::addToQueue(const std::string &message, const LogLevel &level) {
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logQueue.emplace(message, level);
    }
    logCondition.notify_one();
}