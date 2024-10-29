#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <syslog.h>

class Config
{
private:
    std::filesystem::path config_directory;
    std::filesystem::path directory_for_md5_file;
    int interval;

public:
    Config() = default;
    Config(const std::filesystem::path& directory);

    void read(const std::string &filename);
    std::filesystem::path get_directory() const;
    int get_interval() const;
};
