// src/main.cpp
#include "daemon.hpp"
#include "utils.hpp"
#include <signal.h>
#include <iostream>
#include <filesystem>
#include <syslog.h>

int main(int argc, char* argv[]) {
    syslog(LOG_INFO, "%s", std::filesystem:: current_path().c_str());
    std:: string CurPath = std::filesystem:: current_path().c_str();


    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file> <pid_file>" << std::endl;
        return EXIT_FAILURE;
    }

    // Демонизируем процесс
    daemonize();

    // Устанавливаем обработчики сигналов
    signal(SIGHUP, Daemon::handleSignal);
    signal(SIGTERM, Daemon::handleSignal);


    // Запускаем демона
    Daemon::getInstance().run(argv[1], argv[2], CurPath);

    return EXIT_SUCCESS;
}
