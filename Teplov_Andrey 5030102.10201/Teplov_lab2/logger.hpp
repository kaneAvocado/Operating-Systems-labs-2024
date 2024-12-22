#pragma once

#include <string>
#include <iostream>

enum class Status : bool
{
INFO,
ERROR
};

class LoggerClient
{
public:
    static LoggerClient& get_instance()
    {
        static LoggerClient instance;
        return instance;
    }

    void log(Status status, const std::string& message)
    {
        if (status == Status::INFO)
            std::cout << "[CLIENT][INFO] " << message << std::endl;
        else
            std::cout << "[CLIENT][ERROR] " << message << std::endl;
    }

private:
    LoggerClient() = default;
    LoggerClient(const LoggerClient&) = delete;
    LoggerClient& operator=(const LoggerClient&) = delete;
};

class LoggerHost
{
public:
    static LoggerHost& get_instance()
    {
        static LoggerHost instance;
        return instance;
    }

    void log(Status status, const std::string& message)
    {
        if (status == Status::INFO)
            std::cout << "[HOST][INFO] " << message << std::endl;
        else
            std::cout << "[HOST][ERROR] " << message << std::endl;
    }

private:
    LoggerHost() = default;
    LoggerHost(const LoggerHost&) = delete;
    LoggerHost& operator=(const LoggerHost&) = delete;
};
