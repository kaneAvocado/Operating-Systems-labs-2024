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
    _LOG_EMERG = LOG_EMERG,     /* system is unusable */
    _LOG_ALERT = LOG_ALERT,     /* action must be taken immediately */
    _LOG_CRIT = LOG_CRIT,       /* critical conditions */
    _LOG_ERR = LOG_ERR,         /* error conditions */
    _LOG_WARNING = LOG_WARNING,	/* warning conditions */
    _LOG_NOTICE = LOG_NOTICE,	/* normal but significant condition */
    _LOG_INFO = LOG_INFO,       /* informational */
    _LOG_DEBUG = LOG_DEBUG,     /* debug-level messages */
};

const std::string levels[] = { "EMERG", "ALERT", "CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG" };

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
