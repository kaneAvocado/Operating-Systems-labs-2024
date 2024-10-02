#include <cstring>
#include "config.hpp"


Data parse_line(const std::string& line)
{
    char separator = '"';
    auto it1 = std::find(line.begin(), line.end(), separator);

    if (it1 == line.end())
        throw std::runtime_error("line format in config file is incorrect!");

    auto it2 = std::find(it1+1, line.end(), separator);

    if (it2 == line.end())
        throw std::runtime_error("line format in config file is incorrect!");

    std::string folder1 = std::string(it1+1, it2);

    it1 = std::find(it2+1, line.end(), separator);

    if (it1 == line.end())
        throw std::runtime_error("line format in config file is incorrect!");

    it2 = std::find(it1+1, line.end(), separator);

    if (it2 == line.end())
        throw std::runtime_error("line format in config file is incorrect!");

    std::string folder2 = std::string(it1+1,it2);

    int time = std::stoi(std::string(it2+1, line.end()));

    return {folder1, folder2, time};
}

std::vector<Data> Config::read()
{
    std::ifstream in_file(path);
    if (!in_file.is_open())
    {
        syslog(LOG_ERR, "File could not be opened! %s", path.c_str());
        exit(EXIT_FAILURE);
    }
    std::string line;
    std::vector<Data> data;

    while (std::getline(in_file, line))
    {
        try
        {
            data.emplace_back(parse_line(line));
        }
        catch (const std::exception &e)
        {
            syslog(LOG_ERR, "Error while parsing %s: %s", line.c_str(), e.what());
            exit(EXIT_FAILURE);
        }
    }
    in_file.close();
    syslog(LOG_INFO, "Config file %s was read successfully!", path.c_str());
    return data;
}