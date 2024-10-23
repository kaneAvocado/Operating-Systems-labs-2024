#include <string>
#include <iostream>
#include <filesystem>
#include <chrono>
#include "configFile.h"

namespace fs = std::filesystem;

int main()
{
    configFile conf_f = configFile();
    if (!conf_f.readConfigFile("config.conf"))
    {
        return -1;
    }

    const long max_time_live = conf_f.MAX_FILE_AGE_IN_SECONDS;

    std::string folder_1 = conf_f.folder1;
    std::string folder_2 = conf_f.folder2;

    for (const auto& entry : fs::directory_iterator(folder_1))
    {
        std::cout << entry.path() << std::endl;
        fs::file_time_type ftime = fs::last_write_time(entry.path()); 
        
        const auto toNow = fs::file_time_type::clock::now() - ftime;
        const auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(toNow).count();

        if (elapsedSec > max_time_live)
        {
            // если уже был перемещён файл с таким же названием, то он заменяется на его новую версию
            fs::copy(entry.path(), folder_2, fs::copy_options::update_existing); 
            std::error_code errorCode;
            if (!fs::remove(entry.path(), errorCode)) {
                std::cout << errorCode.message() << std::endl;
            }
        }
    }
}