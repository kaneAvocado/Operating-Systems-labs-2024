#include "Config.h"

bool Config::parse_config(const std::string &conf_path) {
    std::ifstream file(conf_path);

    if(!file.is_open())
        return false;
    if (!std::getline(file, folder1))
        return false;
    if (!std::getline(file, folder2))
        return false;
    if (!file >> INTERVAL_SEC)
        return false;

    return true;
}