#include "my_logger/logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
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
                auto now = std::chrono::system_clock::now();
                std::time_t now_c = std::chrono::system_clock::to_time_t(now);
                std::tm now_tm;
                std::tm *ptr = std::localtime(&now_c);
                std::stringstream timeStream ;
                timeStream  << std::put_time(ptr, "%Y-%m-%d %H:%M:%S");
                logFile << "[" << logLevels[static_cast<int>(logQueue.front().second)] << "] ["<<timeStream.str()<<"]" << logQueue.front().first << std::endl;
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