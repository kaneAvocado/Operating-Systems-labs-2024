#include "daemon_utils.h"
#include "Daemon.h"
#include <iostream>
#include <filesystem>


int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Error< more or less than 2 arguments" << std::endl;
        std::cerr << "To use this program, enter: " << argv[0] << " <config_file>" << std::endl;
        return EXIT_FAILURE;
    }

    signal(SIGHUP, Daemon::handleSignal);
    signal(SIGTERM, Daemon::handleSignal);

    ConfigManager* configManager = ConfigManager::getInstance();
    configManager->setConfigPath(argv[1]);

    daemonize();

    Daemon::getInstance()->run();

	return EXIT_SUCCESS;
}