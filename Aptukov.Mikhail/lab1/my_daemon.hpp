#pragma once
#include <cerrno>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/file.h>
#include <syslog.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "monitor.hpp"

class Daemon
{
public:
    static Daemon& get_instance()
    {
        static Daemon instance;
        return instance;
    }
    int daemon_main(const std::filesystem::path&);
private:
    std::filesystem::path config_file;
    const std::string PID_FILE = "/var/run/my_daemon.pid";
};