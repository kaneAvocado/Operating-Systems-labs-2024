#include "config.hpp"

Config::Config(const std::filesystem::path& directory)
{
    config_directory = directory;
}

std::string resolve_path(const std::string &path) {
    std::filesystem::path fs_path(path);
    if (fs_path.is_relative()) {
        return std::filesystem::absolute(fs_path).string();
    }
    return fs_path.string();
}

// Функция для парсинга одной строки конфигурации с учётом кавычек
void parse_config_line(const std::string &line, std::string &key, std::string &value) {
    std::istringstream iss(line);
    std::string temp_key, temp_value;

    bool in_quotes = false;
    bool key_parsed = false;

    std::string current_token;

    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;  // Меняем состояние кавычек
            continue;
        }

        if (in_quotes) {
            current_token += c;
        } else {
            if (std::isspace(c)) {
                if (!current_token.empty()) {
                    if (!key_parsed) {
                        temp_key = current_token;
                        key_parsed = true;
                    } else {
                        temp_value = current_token;
                    }
                    current_token.clear();
                }
            } else if (c == '=') {
                if (!key_parsed) {
                    temp_key = current_token;
                    key_parsed = true;
                }
                current_token.clear();
            } else {
                current_token += c;
            }
        }
    }

    // Обрабатываем последний токен (если он не был добавлен)
    if (!current_token.empty()) {
        if (!key_parsed) {
            temp_key = current_token;
        } else {
            temp_value = current_token;
        }
    }

    key = temp_key;
    value = temp_value;
}

void Config::read(const std::string &filename) {
    auto filename_abs = config_directory/filename; 
    std::ifstream file(filename_abs);
    if (!file) {
        syslog(LOG_ERR, "Failed to open config file %s", filename_abs.c_str());
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string key, value;
        parse_config_line(line, key, value);

        if (key == "directory") {
            try{
                directory_for_md5_file = resolve_path(value);
            } catch(const std::exception &e){
                syslog(LOG_ERR, "Failed in resolve path for file directory: %s", value.c_str());
                exit(EXIT_FAILURE);
            }

            
        } else if (key == "interval") {
            try {
                interval = std::stoi(value);
            } catch (const std::exception &e) {
                syslog(LOG_ERR, "Failed to parse interval value: %s", value.c_str());
                exit(EXIT_FAILURE);
            }
        }
    }

    syslog(LOG_INFO, "Config has been read successfully %s", config_directory.c_str());
}

std::filesystem::path Config::get_directory() const
{
    return directory_for_md5_file;
}

int Config::get_interval() const
{
    return interval;
}