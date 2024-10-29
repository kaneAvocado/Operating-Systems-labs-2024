#include <iostream>
#include <filesystem>
#include "Daemon.h"


int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Misconception: please, enter " << argv[0] << " <config path> " << "<pidfile path> " << std::endl;
        return EXIT_FAILURE;
    }

    signal(SIGHUP, Daemon::handleSignal);
    signal(SIGTERM, Daemon::handleSignal);

    Daemon::getInstance()->run(argv[1], argv[2]);

    return EXIT_SUCCESS;
}