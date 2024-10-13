// src/pidfile.cpp
#include "pidfile.hpp"
#include "utils.hpp"
#include <syslog.h>
#include <fstream>
#include <unistd.h>
#include <string>
#include <signal.h> 
#include <filesystem>

bool createPidFile(const std::string& pidFile) {
    std::ifstream pidFileStream(pidFile);
    if (pidFileStream.is_open()) {
        pid_t existingPid;
        pidFileStream >> existingPid;
        pidFileStream.close();

        if (existingPid > 0 && processExists(existingPid)) {
            syslog(LOG_ERR, "Daemon already running with PID %d. Sending SIGTERM.", existingPid);
            kill(existingPid, SIGTERM);
            // Опционально: подождать завершения старого процесса
            sleep(1);
            if (processExists(existingPid)) {
                syslog(LOG_ERR, "Failed to terminate existing daemon with PID %d.", existingPid);
                return false;
            }
        }
    }

    std::ofstream pidFileOut(pidFile, std::ofstream::trunc);
    if (!pidFileOut.is_open()) {
        syslog(LOG_ERR, "Failed to open PID file for writing: %s", pidFile.c_str());
        return false;
    }

    pid_t pid = getpid();
    pidFileOut << pid << std::endl;
    pidFileOut.close();
    syslog(LOG_NOTICE, "PID file created with PID %d.", pid);
    return true;
}

void removePidFile(const std::string& pidFile) {
    if (remove(pidFile.c_str()) != 0) {
        syslog(LOG_ERR, "Failed to remove PID file: %s", pidFile.c_str());
    } else {
        syslog(LOG_NOTICE, "PID file removed: %s", pidFile.c_str());
    }
}
