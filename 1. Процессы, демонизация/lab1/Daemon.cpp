#include "Daemon.h"

void Daemon::daemonize() {
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int stderr_copy = dup(STDERR_FILENO);

    pid_t pid = fork();

    if (pid < 0) {
        syslog(LOG_ERR, "Fork error: failed to create new process.");
        syslog(LOG_INFO, "Stop initializing...");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        syslog(LOG_INFO, "Forked successfully.");
        exit(EXIT_SUCCESS);
    }
    else {
        umask(0);
        
        if (setsid() < 0) {
            syslog(LOG_ERR, "Error: failed to set new process group.");
            syslog(LOG_INFO, "Stop initializing...");
            exit(EXIT_FAILURE);
        }

        syslog(LOG_INFO, "Set new process group successfully.");
        
        if (chdir("/") < 0) {
            syslog(LOG_ERR, "Error: failed to change directory to root.");
            syslog(LOG_INFO, "Stop initializing...");
            exit(EXIT_FAILURE);
        }

        syslog(LOG_INFO, "Changed directory successfully.");

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        syslog(LOG_INFO, "File descriptors's closed successfully.");

        dup2(stdin_copy, STDIN_FILENO);
        dup2(stdout_copy, STDOUT_FILENO);
        dup2(stderr_copy, STDERR_FILENO);

        syslog(LOG_INFO, "File descriptors's opened successfully.");
    }
}

void Daemon::reload_config(){
    syslog(LOG_INFO, "Config reloading...");
    if (config.parse_config(ABSOLUTE_CONFIG_PATH)) {
        syslog(LOG_INFO, "Config' been reloaded succesfully.");
    }
    else
        syslog(LOG_ERR, "Failed reloading config params.");
}

void Daemon::terminate() {
    syslog(LOG_INFO, "Terminating process...");
    is_terminated = true;
    syslog(LOG_INFO, "Process's been terminated successfully.");
    closelog();
}

void signal_manager(int signal){
    switch (signal) {
    case SIGHUP:
        Daemon::get_instance().reload_config();
        break;
    case SIGTERM:
        Daemon::get_instance().terminate();
        break;
    default:
        break;
    }
}

bool Daemon::check_pid(int &pid) {
    std::ifstream file(PID_PATH);
    if (file.is_open()) {
        if (file >> pid && kill(pid, 0) == 0) {
            file.close();
            return true;
        }
        file.close();
    }
    return false;
}

bool Daemon::update_pidfile() {
    std::ofstream file(PID_PATH);

    if (file.is_open()) {
        file << getpid();
        file.close();
        return true;
    }
    return false;
}

void Daemon::init(const std::string& conf){
    openlog("Open Daemon log.", LOG_NDELAY | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Initializing Daemon...");

    // ABSOLUTE_CONFIG_PATH = fs::absolute(conf);
    ABSOLUTE_CONFIG_PATH = std::filesystem::absolute(conf);
    syslog(LOG_INFO, "Reading config...");
    if(config.parse_config(ABSOLUTE_CONFIG_PATH)) {
        syslog(LOG_INFO, "Config's been read succesfully.");
    }
    else
        syslog(LOG_ERR, "Failed reading config params.");

    syslog(LOG_INFO, "Checking if Daemon's already been started...");
    int pid = 0;
    if (check_pid(pid)) {
        syslog(LOG_INFO, "Daemon's already been started! Killing old process...");
        kill(pid, SIGTERM);
    }
    else
        syslog(LOG_INFO, "Daemon hasn't been started.");

    daemonize();

    syslog(LOG_INFO, "Updating pidfile...");
    if (!update_pidfile()) {
        syslog(LOG_ERR, "Can't open pid file.");
        exit(EXIT_FAILURE);
    }
    else 
        syslog(LOG_INFO, "Pidfile's been updated successfully.");

    syslog(LOG_INFO, "Initializing signals...");
    std::signal(SIGHUP, signal_manager);
    std::signal(SIGTERM, signal_manager);

    syslog(LOG_INFO, "Daemon's been initialized successfully.");
}

void Daemon::copy() {
    std::string subfolder_new = "NEW";
    std::string subfolder_old = "OLD";

    unsigned int lifespan = 180; // 3 min

    if (!std::filesystem::is_directory(config.folder1)) {
        syslog(LOG_WARNING, "folder 1 directory does not exist.");
        return;
    }
    if (!std::filesystem::is_directory(config.folder2)) {
        syslog(LOG_INFO, "folder 2 directory does not exist.");
        return;
    }
    if (!std::filesystem::is_directory(std::filesystem::path(config.folder2) / subfolder_old)) {
        syslog(LOG_INFO, "folder 2 / OLD directory does not exist.");
        return;
    }
    if (!std::filesystem::is_directory(std::filesystem::path(config.folder2) / subfolder_new)) {
        syslog(LOG_INFO, "folder 2 / NEW directory does not exist.");
        return;
    }
    syslog(LOG_INFO, "All folders exist.");

    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(config.folder2) / subfolder_new ))
        std::filesystem::remove(entry.path());
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(config.folder2) / subfolder_old ))
        std::filesystem::remove(entry.path());

    for(const auto& entry: std::filesystem::directory_iterator(config.folder1)) {
        struct stat statbuf;
        stat(entry.path().c_str(), &statbuf); 
        auto file_time = static_cast<std::time_t>(statbuf.st_mtime);
        auto cur_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        
        std::filesystem::path folder2_path = "";
        if ((cur_time - file_time) >= lifespan) {
            folder2_path = std::filesystem::path(config.folder2) / subfolder_old / entry.path().filename();
        }
        else {
            folder2_path = std::filesystem::path(config.folder2) / subfolder_new / entry.path().filename();
        }

        std::filesystem::copy_file(entry.path(), folder2_path, std::filesystem::copy_options::skip_existing);
    }       
}

void Daemon::run(){
    while(!is_terminated){
        syslog(LOG_INFO, "Run Daemon...");
        copy();
        sleep(config.INTERVAL_SEC);
    }
}
