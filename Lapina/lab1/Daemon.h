#ifndef DAEMON_H
#define DAEMON_H

#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>
#include <csignal>
#include <sys/stat.h>

#include <chrono>
#include <unistd.h>

#include <filesystem>
#include <syslog.h>

#include "configFile.h"


class Daemon {

private:
    configFile d_config;
    std::string d_configAbsolutePath = "/home/lapinaolga/lab_Operation_system/Lapina/lab1";
    std::string d_pidPath = "/var/run/lab1.pid";
    bool d_sigTerminate = false;

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
    friend void signalHandler(int signal);

    // function for signals
    void forSIGTERM();
    void forSIGHUP(); 


public:
    // getting a daemon instance
    static Daemon& getInstanceDaemon();

    // initializing the daemon
    void initDaemon(const std::string& config_path);

    // starting the daemon
    void runDaemon();
};

#endif
