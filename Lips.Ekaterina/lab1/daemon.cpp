#include "daemon.h"
#include <unistd.h>
#include <csignal>
#include <syslog.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <filesystem>


// initialization of singleton
Daemon Daemon::daemon_instance;

// check if string can be converted to int
bool string_to_uint(const std::string& str, unsigned int& result) {
    try {
        result = std::stoul(str); // try convert string в int
        return true; 
    } catch (const std::invalid_argument& e) {
        return false; // string is not a number
    } catch (const std::out_of_range& e) {
        return false; // number is outside of int
    }
}

// callback for signal handle
static void signal_handler(int sign) {
    switch(sign) {
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

void Daemon::daemonize(void) {
    pid_t pid;

    // store duplicates of std descriptors
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int stderr_copy = dup(STDERR_FILENO);

    // create child process
    pid = fork();

    // close parent process
    if (pid < 0) {
        syslog(LOG_WARNING, "Invalid fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        syslog(LOG_INFO, "New process created successfully, parent process can be terminated");
        exit(EXIT_SUCCESS);
    }

    umask(0); 

    // set new process group (child process becomes session leader)
    if (setsid() < 0) {
        syslog(LOG_WARNING, "Can't set group");
        exit(EXIT_FAILURE);
    }

    // change working dir
    if (chdir("/") < 0) {
        syslog(LOG_WARNING, "Can't change directory");
        exit(EXIT_FAILURE);
    }

    // close all opened file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x>=0; x--)
        close (x);

    // open file std descriptors
    dup2(stdin_copy, STDIN_FILENO);
    dup2(stdout_copy, STDOUT_FILENO);
    dup2(stderr_copy, STDERR_FILENO);
}

void Daemon::parse_config(void) {

    interval_time = DEFAULT_INTERVAL_TIME;
    file_lifetime = DEFAULT_FILE_LIFETIME;

    std::ifstream config_file(config_path);

    // open config file
    if (!config_file.is_open()) {
        syslog(LOG_WARNING, "Invalid config file: file can't be opened");
        return;
    }

    std::string line;

    // get path to folders
    for (int i=0; i<=1; i++) {
        if (std::getline(config_file, line)) {
            std::filesystem::path path(line);
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) folders[i] = line;
            else {
                syslog(LOG_WARNING, "Config file contains incorrect data (folder doesn't exists), please reconfig daemon");
                return;
            }
            syslog(LOG_INFO, "--> folder%i - %s", i+1, folders[i].c_str());
        }
        else {
            syslog(LOG_WARNING, "Invalid config file: not enough data");
            return;
        }
    }

    // get file lifetime
    if (!std::getline(config_file, line) or !string_to_uint(line, file_lifetime))
        syslog(LOG_WARNING, "Config file contains incorrect data (not found file lifetime, use default lifetime)");
    syslog(LOG_INFO, "--> file lifetime - %u minutes", file_lifetime);

    // get interval lifetime
    if (!std::getline(config_file, line) or !string_to_uint(line, interval_time))
        syslog(LOG_WARNING, "Config file contains incorrect data (not found interval time, use default interval time)");
    syslog(LOG_INFO, "--> interval time - %u seconds", interval_time);

    valid_config_flag = true;

    syslog(LOG_INFO, "Config loaded successfully");
}

void Daemon::check_pid(void) {
    std::ifstream pid_file(PID_FILE_PATH);
    syslog(LOG_INFO, "Check pid file");
    if (pid_file.is_open()) {
        int other_pid = 0;
        if (pid_file >> other_pid) {
            // if process exist - kill it
            if (kill(other_pid, 0) == 0) {
                syslog(LOG_INFO, "Another instance of daemon in running, terminate it..");
                kill(other_pid, SIGTERM); 
            }
        }
    }
}

void Daemon::set_pid(void) {
    std::ofstream pid_file(PID_FILE_PATH.c_str());
    if (!pid_file.is_open()) {
        syslog(LOG_WARNING, "Can't write to pid file, exit");
        exit(EXIT_FAILURE);
    }

    pid_file << getpid();
}

void Daemon::inition(std::string config_filename) {
    char buf[PATH_MAX];
    getcwd(buf, sizeof(buf)); // full path of the current directory
    config_path = buf;
    config_path += "/" + config_filename;

    openlog("Daemon", LOG_NDELAY | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Daemon initialization");

    
    check_pid(); 

    syslog(LOG_INFO, "Daemonification");
    daemonize();

    set_pid();

    // set signal handler
    syslog(LOG_INFO, "Set signal handlers");
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);

    parse_config();
}

void Daemon::reload_config(void) {
    syslog(LOG_INFO, "Config reloading..");
    valid_config_flag = false;
    parse_config();
}

void Daemon::terminate(void) {
    syslog(LOG_INFO, "Process terminated");
    running_flag = false;
    closelog();
}

void Daemon::run(void) {
    syslog(LOG_INFO, "Daemon working..");
    while (running_flag) {
        move_files();
        sleep(interval_time);
    }
}

void Daemon::move_files(void) {
    syslog(LOG_INFO, "Moving files..");

    if (!valid_config_flag) {
        syslog(LOG_WARNING, "Invalid config file");
        return;
    }

    auto time_now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    struct stat buff;
    std::vector<std::filesystem::path> move_files1, move_files2;
    std::vector<std::vector<std::filesystem::path>> all_move_files;

    // get file to move from folder1
    for (const auto &elem : std::filesystem::directory_iterator(folders[0])) {
        if (!elem.is_directory()) {
            stat(elem.path().c_str(), &buff);
            if ((time_now - timelocal(localtime(&buff.st_mtime))) / 60 > file_lifetime)
                move_files1.push_back(elem.path());
        }
    }
    all_move_files.push_back(move_files1);

    // get file to move from folder2
    for (const auto &elem : std::filesystem::directory_iterator(folders[1])) {
        if (!elem.is_directory()) {
            stat(elem.path().c_str(), &buff);
            if ((time_now - timelocal(localtime(&buff.st_mtime))) / 60 < file_lifetime)
                move_files2.push_back(elem.path());
        }
    }
    all_move_files.push_back(move_files2);

    // move file (change path to file)
    for (int i=0; i<=1; i++) {
        for (auto &file : all_move_files[i])
            std::filesystem::rename(file, folders[(i+1)%2] / file.filename());
    }

    syslog(LOG_INFO, "Files moved successfully");
}