#pragma once
#include <vector>
#include <unordered_map>
#include <functional>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <thread>
#include <string>
#include <chrono>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <tuple>
#include <signal.h>
#include"Config.h"

class Daemon {
public:
    static Daemon &get_instance() {
        static Daemon instance;
        return instance;
    };

    void init(const std::string& config);
    void run();

private:
    void daemonize();
    void reload_config();
    void terminate();
    void copy();

    Config config;

    bool is_terminated = false;
    bool check_pid(int &pid);
    bool update_pidfile();

    friend void signal_manager(int signal);

    const std::string PID_PATH = "/var/run/lab1.pid";
    std::string ABSOLUTE_CONFIG_PATH = "";

    Daemon() = default;
    Daemon(const Daemon &) = delete;
    Daemon &operator=(const Daemon &) = delete;
    Daemon(Daemon &&) = delete;
    Daemon &operator=(Daemon &&) = delete;
};