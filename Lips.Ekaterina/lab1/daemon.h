#pragma once
#include <string>
#include <chrono>
#include <vector>


class Daemon {
private:
    
    const std::string PID_FILE_PATH = "/var/run/lab1.pid";  // path to pid file 
    const unsigned int DEFAULT_INTERVAL_TIME = 30;  // default time interval (seconds) 
    const unsigned int DEFAULT_FILE_LIFETIME = 10;  // default lifetime before moving to another folder (minutes)

    unsigned int interval_time = DEFAULT_INTERVAL_TIME;  // current time interval (seconds)
    unsigned int file_lifetime = DEFAULT_FILE_LIFETIME; // current lifetime before moving to another folder (minutes)

    std::vector<std::string> folders{ "", "" };  // paths to folders

    std::string config_path;  // path to config file

    bool running_flag = true;  // to check if the daemon is running
    bool valid_config_flag = false;  // to check if config is valid
    

    // variable for singleton pattern
    static Daemon daemon_instance;
    
    void daemonize(void);  // function to transform process into deamon
    
    void parse_config(void);  // function to parse config file
    
    void check_pid(void);  // function to check if daemon is already running

    void set_pid(void);  // function to write pid to pid file

    void move_files(void);  // function to move files between folders

    
    Daemon() {} // private constructor 
    
    // block copy and move constructors
    Daemon(const Daemon& root) = delete;
    Daemon& operator=(const Daemon&) = delete;

public:

    void inition(std::string configFilePath);  // initialization function
    
    static Daemon &get_instance(void) { return daemon_instance; }  // get instance function
    
    void reload_config(void); // function to reload config

    void terminate(void); // function to terminate daemon work

    void run(void); // function to start daemon work

};