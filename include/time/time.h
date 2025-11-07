#ifndef TIME_H
#define TIME_H
#include <stdint.h>
#include <functional>
#include <chrono>
#include <thread>
class clock{
public:
    void sleep_for(int duration_ms, std::function<void()> callback){
        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
            callback();
        }
    }
};


#endif // TIME_H
