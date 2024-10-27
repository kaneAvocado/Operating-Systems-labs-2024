#include "PidfileManager.h"

#include <syslog.h>
#include <fstream>
#include <unistd.h>
#include <string>
#include <signal.h> 
#include <filesystem>

#include <iostream>

namespace fs = std::filesystem;
PidfileManager* PidfileManager::instance_ptr = nullptr;

void PidfileManager::setPidFilePath(std::string path) {
    pidFilePath = path;
}

PidfileManager* PidfileManager::getInstance()
{
    if (!instance_ptr) {
        instance_ptr = new PidfileManager();
    }
    return instance_ptr;
}

bool PidfileManager::create() {
    if (isExistingPidRunning()) {
        if (!terminateExistingProcess()) {
            return false;
        }
    }
    return writePidToFile();
}

void PidfileManager::remove() const {
    if (fs::remove(pidFilePath)) {
        syslog(LOG_NOTICE, "PID file removed: %s", pidFilePath.c_str());
    }
    else {
        syslog(LOG_ERR, "Failed to remove PID file: %s", pidFilePath.c_str());
    }
}

bool PidfileManager::isExistingPidRunning() const {
    std::ifstream pidFileStream(pidFilePath);
   
    if (pidFileStream.is_open()) {
        pid_t existingPid;
        pidFileStream >> existingPid;
        pidFileStream.close();

        if (existingPid > 0 && processExists(existingPid)) {
            syslog(LOG_ERR, "Daemon already running with PID %d.", existingPid);
            return true;
        }
    }
    return false;
}

bool PidfileManager::terminateExistingProcess() const {
    std::ifstream pidFileStream(pidFilePath);
    pid_t existingPid;
    pidFileStream >> existingPid;

    if (kill(existingPid, SIGTERM) == 0) {
        sleep(1);
        if (!processExists(existingPid)) {
            syslog(LOG_NOTICE, "Successfully terminated daemon with PID %d.", existingPid);
            return true;
        }
    }

    syslog(LOG_ERR, "Failed to terminate existing daemon with PID %d.", existingPid);
    return false;
}

bool PidfileManager::writePidToFile() const {
    std::ofstream pidFileOut(pidFilePath, std::ofstream::trunc);
    if (!pidFileOut.is_open()) {
        syslog(LOG_ERR, "Failed to open PID file for writing: %s", pidFilePath.c_str());
        return false;
    }

    pidFileOut << getpid() << std::endl;
    pidFileOut.close();
    syslog(LOG_NOTICE, "PID file created with PID %d.", getpid());
    // std::cerr << "pid " << getpid() << std::endl;
    return true;
}

bool PidfileManager::processExists(pid_t pid) const {
    // Èìïëåìåíòàöèÿ ïðîâåðêè ñóùåñòâîâàíèÿ ïðîöåññà
    return (kill(pid, 0) == 0);
}
