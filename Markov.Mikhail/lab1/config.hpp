#pragma once

#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <csignal>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <exception>

#include "data.hpp"

class Config
{
private: 
    std::filesystem::path path;

public:
    Config() = default;
    Config(const std::string &filename) 
    {
        path = std::filesystem::path(filename);

        if (path.is_relative())
        {
            std::filesystem::path cur_path = std::filesystem::current_path();
            cur_path = cur_path / filename;
            path = std::filesystem::absolute(cur_path);
        }
        if (!std::filesystem::exists(path))
        {
            syslog(LOG_ERR, "Config file %s does not exist! ", path.c_str());
        }
    }

    std::vector<Data> read();
};