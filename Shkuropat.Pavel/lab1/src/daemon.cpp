// src/daemon.cpp
#include "daemon.hpp"
#include "pidfile.hpp"
#include "utils.hpp"
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <dirent.h>
#include <cstring>
#include <filesystem>
#include <errno.h>

namespace fs = std::filesystem;

// Инициализация переменных класса
Daemon& Daemon::getInstance() {
    static Daemon instance;
    return instance;
}

Daemon::Daemon()

    : running(true), interval(20), newConfPath()
{}

Daemon::~Daemon() {}

void Daemon::run(const std::string& configFile, const std::string& pidFile, const std::string& current_path) {
    openlog("daemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE, "Daemon started.");
    newConfPath = current_path;
    syslog(LOG_INFO, "%s", current_path.c_str());
    syslog(LOG_INFO, "%s", newConfPath.c_str());
    // Создание PID-файла
    if (!createPidFile((newConfPath/pidFile))) {
        syslog(LOG_ERR, "Не удалось создать PID-файл, завершение работы.");
        exit(EXIT_FAILURE);
    }

    // Загрузка конфигурации
    loadConfig((newConfPath/configFile));

    // Основной цикл работы демона
    while (running) {
        performActions();
        sleep(interval);
    }

    // Очистка и завершение
    removePidFile((newConfPath/pidFile));
    syslog(LOG_NOTICE, "Daemon terminated.");
    closelog();
}

void Daemon::handleSignal(int signum) {
    if (signum == SIGHUP) {
        syslog(LOG_NOTICE, "Получен сигнал SIGHUP, перезагрузка конфигурации.");
        Daemon::getInstance().loadConfig((Daemon::getInstance().newConfPath/Daemon::getInstance().configPath));
    } else if (signum == SIGTERM) {
        syslog(LOG_NOTICE, "Получен сигнал SIGTERM, завершение работы.");
        Daemon::getInstance().running = false;
    } else {
        syslog(LOG_WARNING, "Получен неизвестный сигнал: %d", signum);
    }
}

void Daemon::loadConfig(const std::string& configFile) {
    syslog(LOG_NOTICE, "Загрузка конфигурации из %s", configFile.c_str());
    configPath = configFile;
    config.clear();

    std::ifstream file(configFile);
    if (!file.is_open()) {
        syslog(LOG_ERR, "Не удалось открыть конфигурационный файл: %s", configFile.c_str());
        return;
    }

    std::string folder, ignFile;
    while (file >> folder >> ignFile) {
        config.emplace_back((Daemon::getInstance().newConfPath/folder), ignFile);
    }
    file.close();
}

void Daemon::performActions() {
    for (const auto& [folder, ignFile] : config) {
        DIR* dir = opendir(folder.c_str());
        if (!dir) {
            syslog(LOG_ERR, "Не удалось открыть директорию: %s", folder.c_str());
            continue;
        }

        bool ignore = false;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ignFile.c_str()) == 0) {
                ignore = true;
                break;
            }
        }
        closedir(dir);

        if (!ignore) {
            syslog(LOG_NOTICE, "Удаление содержимого директории: %s", folder.c_str());
            deleteFolderContents(folder);
        }
    }
}

void Daemon::deleteFolderContents(const std::string& folder) {
    for (const auto& entry : fs::directory_iterator(folder)) {
        try {
            fs::remove_all(entry.path());
            syslog(LOG_NOTICE, "Удалено: %s", entry.path().c_str());
        } catch (const fs::filesystem_error& e) {
            syslog(LOG_ERR, "Ошибка при удалении %s: %s", entry.path().c_str(), e.what());
        }
    }
}
