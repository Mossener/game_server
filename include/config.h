#ifndef CONFIG_H
#define CONFIG_H
#include <string>
struct Config {
public:
    static const int SERVER_PORT = 10084;
    static const size_t THREAD_POOL_SIZE = 4;
    static constexpr const char* LOG_PATH = LOGGER_PATH "/app.log";
    static const int MAX_EVENTS = 10;
};
#endif