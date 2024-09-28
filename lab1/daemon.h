#ifndef DAEMON_H
#define DAEMON_H

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include <libconfig.h++>
#include <optional>

#include "logger.h"

#define LOG_INDENT "                   "

class Daemon {

protected:
    Daemon() = default;
    ~Daemon() = default;

public:
    Daemon(const Daemon&) = delete;
    Daemon(Daemon&&) = delete;
    Daemon& operator=(Daemon const&) = delete;
    Daemon& operator=(Daemon &&) = delete;
    static Daemon& Instance() {
        static Daemon instance;
        return instance;
    }
    static Daemon* InstancePtr() {
        return &Instance();
    }
    void Start();
    void ReadConfig();
    void Proccessing();

private:
    template<typename TypeValue>
    TypeValue getSettingValue(const libconfig::Setting& setting, const std::string& key);
    uintmax_t getFolderSize(const std::filesystem::path& folderPath);
    void clearFolder(const std::filesystem::path& folderPath);
    void checkDirectoryExists(const std::string& dirPath);

private:
    std::string configPath;
    std::pair<std::string, std::string> folders;
    int time;
    std::string logFile;
};

#endif // DAEMON_H
