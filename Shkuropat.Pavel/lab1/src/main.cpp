// src/main.cpp
#include "daemon.hpp"
#include "utils.hpp"
#include <signal.h>
#include <iostream>

int main(int argc, char* argv[]) {
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
    Daemon::getInstance().run(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
