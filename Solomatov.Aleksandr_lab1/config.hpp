#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <syslog.h>

class Config
{
private:
    std::filesystem::path directory;
    int interval;

public:
    Config() = default;
    Config(const std::string &filename);

    void read();
    std::filesystem::path get_directory() const;
    int get_interval() const;
};
