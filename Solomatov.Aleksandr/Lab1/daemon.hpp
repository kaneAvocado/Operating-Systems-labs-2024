#pragma once

#include <vector>
#include <chrono>
#include <filesystem>
#include <csignal>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <thread>
#include "config.hpp"
#include <openssl/evp.h>


class Daemon
{
public:
    static Daemon &get_instance()
    {
        static Daemon instance;
        return instance;
    };
    void run(const std::filesystem::path &, const std::string &);
    void open_config_file(const std::string &filename);

private:
    Config config;

    volatile sig_atomic_t got_sighup = 0;
    volatile sig_atomic_t got_sigterm = 0;

    void create_md5_file(const std::filesystem::path &);
    void create_pid_file();
    void daemonize();

    friend void signal_handler(int sig);

    Daemon() = default;
    Daemon(const Daemon &) = delete;
    Daemon(Daemon &&) = delete;
    Daemon &operator=(const Daemon &) = delete;
    Daemon &operator=(Daemon &&) = delete;
};