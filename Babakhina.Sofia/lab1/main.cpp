#include <iostream>

#include "Daemon.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Uncorrent count of arguments! Programm need only one argument: path to the logger config!" << std::endl;
        return EXIT_FAILURE;
    }
    else {
        std::string conf = argv[1];
        // std::string conf = "Config.txt"; // name.txt if conf exist in root dir, relative path if not
        std::string configPath = std::string(conf);
        Daemon::get_instance().init(configPath);
        Daemon::get_instance().run();
        return EXIT_SUCCESS;
    }
}