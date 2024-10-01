#pragma once

#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <thread>
#include "config.hpp"

class Daemon
{
public:
    static Daemon &get_instance()
    {
        static Daemon instance;
        return instance;
    };
    void run(const std::filesystem::path &, const std::string &);
    void open_config_file();

private:
    std::filesystem::path current_path;
    Config config;
    std::vector<Data> table;
    std::vector<std::chrono::time_point<std::chrono::steady_clock>> time_points;

    void replace_folder(const Data&);
    void create_pid_file();
    void daemonize();
    void set_data(const std::vector<Data> &);

    Daemon() = default;
    Daemon(const Daemon &) = delete;
    Daemon(Daemon &&) = delete;
    Daemon &operator=(const Daemon &) = delete;
    Daemon &operator=(Daemon &&) = delete;
};