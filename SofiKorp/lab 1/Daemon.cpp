#include "Daemon.h"
#include <syslog.h>
#include <chrono>
#include <linux/limits.h>

Daemon* Daemon::instance = nullptr;

Daemon* Daemon::getInstance()
{
    if (!Daemon::instance) {
        Daemon::instance = new Daemon();
    }
    return Daemon::instance;
}

void Daemon::handleSignal(int signum)
{
    if (signum == SIGHUP) {
        syslog(LOG_NOTICE, "Signal SIGHUP: reload configuration");
        Daemon::getInstance()->readConfig();
    }
    else if (signum == SIGTERM) {
        syslog(LOG_NOTICE, "Signal SIGTERM: stop workin.");
        Daemon::getInstance()->end_program = true;
    }
    else {
        syslog(LOG_WARNING, "Undefined signal: %d", signum);
    }
}

void Daemon::run(std::string configPath, std::string pidfilePath)
{
    openlog("daemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE, "Daemon started.");

    char buffer[PATH_MAX];
    if (getcwd(buffer, PATH_MAX) == nullptr) {
        syslog(LOG_ERR, "Error getting current dirrectory");
        exit(EXIT_FAILURE);
    }
    configParams.currentDir = std::string(buffer);

    readConfig(configPath);
    demonizeProcess();
    createPid(pidfilePath);

    while (!end_program) {
        check_working_folders();
        sleep(static_cast<unsigned int>(std::chrono::seconds(configParams.interval).count())); 
    }

    removePid();
    syslog(LOG_NOTICE, "Daemon terminated.");
    closelog();
}

void Daemon::readConfig(std::string configPath)
{
    this->configPath = complementPath(configPath);
    readConfig();
}

void Daemon::readConfig()
{
    std::ifstream reader(configPath);

    if (!reader.is_open()) {
        syslog(LOG_ERR, "Cannot open config file: %s", configPath.c_str());
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(reader, line)) {
        std::string valueName, valueMean;
        
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos)
            continue;

        valueName = line.substr(0, equalPos);
        valueName.erase(valueName.find_last_not_of(" \t") + 1); 


        valueMean = line.substr(equalPos + 1);
        valueMean.erase(0, valueMean.find_first_not_of(" \t")); 
        
        
        if (valueName == "working folder 1") {
            //std::cerr << "working folder 1 " << complementPath(valueMean);
            configParams.workingFolder1 = complementPath(valueMean);
            continue;
        }
        if (valueName == "working folder 2") {
            //std::cerr << "working folder 2 " << complementPath(valueMean);
            configParams.workingFolder2 = complementPath(valueMean);
            continue;
        }
        if (valueName == "interval") {
            try {
                configParams.interval = std::stoi(valueMean);
                continue;
            }
            catch (const std::exception& e){
                syslog(LOG_ERR, "Error converting the interval parameter: %s", e.what());
                exit(EXIT_FAILURE);
            }
        }
    }
}

std::string Daemon::complementPath(std::string path)
{
    path = trim(path);
    if (path[0] != '/') {
        path = configParams.currentDir + "/" + path;
    }

    while (std::iscntrl(path[path.size() - 1])) {
        path.erase(path.size() - 1);
    }
    return path;
}


std::string Daemon::trim(const std::string str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

void Daemon::demonizeProcess()
{
    pid_t pid = fork();
    
    if (pid < 0) {
        syslog(LOG_ERR, "Cannot create new process");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        syslog(LOG_NOTICE, "End parent process");
        exit(EXIT_SUCCESS);
    }

    pid_t sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Cannot create sid");
    }

    // Ignore signals
    signal(SIGCHLD, SIG_IGN);
    //signal(SIGHUP, SIG_IGN);

    // Second fork to prevent detaching from the terminal
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    //Set mask
    umask(0);

    // Change dir
    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    // Redirection
    
    close(STDIN_FILENO);
    open("/dev/null", O_RDONLY); 
    close(STDOUT_FILENO);
    open("/dev/null", O_RDWR);  
    close(STDERR_FILENO);
    open("/dev/null", O_RDWR);  
}

bool Daemon::createPid(std::string pidfilePath)
{
    this->pidfilePath = complementPath(pidfilePath);
    return createPid();
}

bool Daemon::createPid() {
    
    // check is exiting pid running
    std::ifstream pidFileStream(pidfilePath);
    if (pidFileStream.is_open()) {
        pid_t existingPid;
        pidFileStream >> existingPid;
        pidFileStream.close();

        if (existingPid > 0 && (kill(existingPid, 0) == 0)) {
            syslog(LOG_ERR, "Daemon already running with PID %d.", existingPid);
            // try to terminate exiting process
            if (kill(existingPid, SIGTERM) == 0) {
                sleep(1);
                if (kill(existingPid, 0) == 0) {
                    syslog(LOG_ERR, "Failed to terminate existing daemon with PID %d.", existingPid);
                    return false;
                }
                syslog(LOG_NOTICE, "Successfully terminated daemon with PID %d.", existingPid);
            }
        }
    }

    // try to write into pid file
    std::ofstream pidFileOut(pidfilePath, std::ofstream::trunc);
    if (!pidFileOut.is_open()) {
        syslog(LOG_ERR, "Failed to open PID file for writing: %s", pidfilePath.c_str());
        return false;
    }

    pid_t pid = getpid();
    pidFileOut << pid << std::endl;
    pidFileOut.close();
    syslog(LOG_NOTICE, "PID file created with PID %d.", pid);
    return true;
}

void Daemon::removePid() const
{
    if (std::filesystem::remove(pidfilePath)) {
        syslog(LOG_NOTICE, "PID file removed: %s", pidfilePath.c_str());
    }
    else {
        syslog(LOG_ERR, "Failed to remove PID file: %s", pidfilePath.c_str());
    }
}

void Daemon::check_working_folders() {

    if (!std::filesystem::exists(configParams.workingFolder1)) {
        syslog(LOG_ERR, "Cannot find working folder 1: %s", configParams.workingFolder1.c_str());
        exit(EXIT_FAILURE);
    }

    if (!std::filesystem::exists(configParams.workingFolder2)) {
        syslog(LOG_ERR, "Cannot find working folder 2: %s", configParams.workingFolder2.c_str());
        exit(EXIT_FAILURE);
    }

    // Clean workingFolder2
    for (const auto& entry : std::filesystem::directory_iterator(configParams.workingFolder2)) {
        std::filesystem::remove_all(entry.path());
    }

    // Copy files "*.bk" from workingFolder1 to workingFolder2
    for (const auto& entry : std::filesystem::directory_iterator(configParams.workingFolder1)) {
        if (entry.is_regular_file() && entry.path().extension() == ".bk") {
            std::filesystem::copy_file(entry.path(), configParams.workingFolder2 + "/" + entry.path().filename().string(), 
                std::filesystem::copy_options::overwrite_existing);
        }
    }
}
