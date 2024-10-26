#include "logger.h"

namespace Logger {

Logger::Logger(std::string name) : name(std::move(name)) {
    openlog(this->name.c_str(), LOG_PID | LOG_CONS, LOG_USER);
}

std::string Logger::formatLog(LogLevel level, const std::string& message) {
    std::ostringstream oss;
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << " ["
        << levels[static_cast<int>(level)] << "] " << message;
    return oss.str();
}

void Logger::logMessage(LogLevel level, const char* format, ...) {
    std::lock_guard<std::mutex> guard(log_mutex);

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string message = formatLog(level, buffer);
    syslog(static_cast<int>(level), "%s", message.c_str());
}

void Logger::resetName(const std::string& name) {
    closelog();
    this->name = name;
    openlog(this->name.c_str(), LOG_PID | LOG_CONS, LOG_USER);
}

void Logger::logToFile(const std::string& filePath, LogLevel level, const char* format, ...) {
    std::lock_guard<std::mutex> lock(log_mutex);

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string message = formatLog(level, buffer);

    std::ofstream logFile(filePath, std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::stringstream ss;
        ss << "Задан неверный путь к файлу: \n" << filePath << ".\n";
        throw std::runtime_error(ss.str());
    }
}

}
