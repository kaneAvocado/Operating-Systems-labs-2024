#ifndef DAEMON_H
#define DAEMON_H

#include <string>
#include <iostream>

#include <chrono>
#include <unistd.h>

#include <filesystem>
#include <syslog.h>

#include "configFile.h"


class Daemon {

private:
    static Daemon* instance;
    configFile config;
    std::string configAbsolutePath;
    std::string pidPath = "";

    // default c-tor and d-tor
    Daemon() = default;
    ~Daemon() = default;

    // Delete the copy constructor and assignment operator
    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;
    Daemon(Daemon&&) = delete;
    Daemon& operator=(Daemon&&) = delete;


    // the function of demonizing the process
    void daemonize();

    // command for Daemon
    void commandDaemon();

    // signal handler
    void signalHandler(int signal);

    // function for signals
    void forSIGTERM();
    void forSIGHUP(); 

    bool sigTerminate = false;


public:
    static Daemon& getInstanceDaemon()
    {
        if (!instance) {
            instance = new Daemon();
        }
        return *instance;
    }

    static Daemon& initDaemon(const std::string& config_path)
    {
        if (!instance) {
            instance = new Daemon();
        }
        return *instance;
    }

    void runDaemon();
};

#endif
