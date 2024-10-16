#include <cstring>
#include "config.hpp"


Data parse_line(const std::string& line)
{
    std::vector<std::string> words;
    std::string current_word;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == '"')
        {
            in_quotes = !in_quotes;
            if (!in_quotes && !current_word.empty())
            {
                words.push_back(current_word);
                current_word.clear(); 
            }
        }
        else if (std::isspace(c))
        {
            if (!in_quotes)
            {
                if (!current_word.empty())
                {
                    words.push_back(current_word);
                    current_word.clear(); 
                }
            }
            else
                current_word += c;
        }
        else
            current_word += c;
    }

    if (!current_word.empty())
        words.push_back(current_word);

    if(words.size() == 3)
    {
        syslog(LOG_INFO, "line:%s,%s,%s", words[0].c_str(), words[1].c_str(), words[2].c_str()) ;
        return {words[0], words[1], std::stoi(words[2])};
    }
    else    
    {
        throw(std::runtime_error("incorrect format"));
    }
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
        }
    }
    in_file.close();
    syslog(LOG_INFO, "Config file %s was read successfully!", path.c_str());
    return data;
}