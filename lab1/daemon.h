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
#include <signal.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <cstring>

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
    void ConnectSignals();
    void termLog();
    void hupReadConfig();
    static void termHandler(int signum, siginfo_t *info, void *ctx);
    static void hupHandler(int signum, siginfo_t *info, void *ctx) {
        readConfig = true;
    }


private:
    std::string configPath;
    std::pair<std::string, std::string> folders;
    int time;
    std::string logFile;
    struct sigaction term_handler;
    struct sigaction hup_handler;
    static std::atomic<bool> stopDaemon;
    static std::atomic<bool> readConfig;
    static std::mutex logMutex;
    static std::mutex configMutex;
    static std::queue<std::string> logQueue;
};

#endif // DAEMON_H
