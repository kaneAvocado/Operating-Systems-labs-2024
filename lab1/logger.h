#include <syslog.h>
#include <string>
#include <chrono>
#include <mutex>
#include <iostream>
#include <sstream>
#include <cstdarg>
#include <vector>
#include <iomanip>
#include <fstream>

namespace Logger {
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

const std::string levels[] = {"DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};

class Logger {
public:

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    static Logger& getInstance() {
        static Logger instance("DaemonLoger");
        return instance;
    }
    static Logger* InstancePtr() {
        return &getInstance();
    }

    // Функции логирования
    void logMessage(LogLevel level, const char* format, ...);
    void resetName(const std::string& new_name);
    void logToFile(const std::string& filePath, LogLevel level, const char* format, ...);

private:
    Logger(std::string name);

    std::string formatLog(LogLevel level, const std::string& message);

    std::mutex log_mutex;
    std::string name;
};
}
