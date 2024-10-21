#pragma once
#include <string>
#include <filesystem>
#include <fstream>

class Config {
public:
    std::string folder1;
    std::string folder2;
    int INTERVAL_SEC;

    bool parse_config(const std::string &conf_path);
};
