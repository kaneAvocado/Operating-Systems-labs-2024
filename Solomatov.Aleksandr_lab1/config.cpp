#include "config.hpp"

Config::Config(const std::string &filename)
{
    read();
}

void Config::read()
{
    std::ifstream file("config.cfg");
    if (!file)
    {
        syslog(LOG_ERR, "Failed to open config file");
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value))
        {
            if (key == "directory")
                directory = value;
            else if (key == "interval")
                interval = std::stoi(value);
        }
    }

    syslog(LOG_INFO, "Config has read");
}
std::filesystem::path Config::get_directory() const
{
    return directory;
}

int Config::get_interval() const
{
    return interval;
}