#include <iostream>

#include "Daemon.h"

int main(int argc, char** argv) 
{
    if (argc != 2) {
        std::cout << "Enter only the path of the configuration file" << std::endl;
        return EXIT_FAILURE;
    }
    else 
    {

        std::string configPath = argv[1];
        
        Daemon::getInstanceDaemon().initDaemon(configPath);
        Daemon::getInstanceDaemon().runDaemon();
        return EXIT_SUCCESS;
    }
}