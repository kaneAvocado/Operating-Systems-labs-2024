#include "my_daemon.hpp"

int read_pid(const char *pid_file)
{
    std::ifstream f(pid_file);
    if (!f.is_open())
    {
        syslog(LOG_DEBUG, "read_pid: fopen failed");
        return -1;
    }

    int pid;
    f >> pid;

    if (f.fail())
    {
        syslog(LOG_DEBUG, "read_pid: fscanf failed");
        return -1;
    }

    syslog(LOG_DEBUG, "read_pid: pid = %d", pid);
    return pid;
}

int check_pid(const char *pid_file)
{
    int pid = read_pid(pid_file);
    if (pid == -1)
    {
        syslog(LOG_DEBUG, "check_pid: read_pid failed");
        return -1;
    }

    errno = 0;
    if (kill(pid, 0) < 0 && errno == ESRCH)
    {
        syslog(LOG_DEBUG, "check_pid: no such process");
        return 0;
    }

    syslog(LOG_DEBUG, "check_pid: pid = %d", pid);
    return pid;
}

int write_pid(const char *pid_file)
{
    int fd = open(pid_file, O_RDWR | O_CREAT, 0644);
    if (fd == -1)
    {
        syslog(LOG_DEBUG, "write_pid: open failed");
        return -1;
    }

    FILE *f = fdopen(fd, "w");
    if (f == nullptr)
    {
        syslog(LOG_DEBUG, "write_pid: fdopen failed");
        close(fd);
        return -1;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) == -1)
    {
        syslog(LOG_DEBUG, "write_pid: flock failed");
        fclose(f);
        return -1;
    }

    fprintf(f, "%d\n", getpid());
    fflush(f);

    if (flock(fd, LOCK_UN) == -1)
    {
        syslog(LOG_DEBUG, "write_pid: unlock failed");
        close(fd);
        return -1;
    }

    close(fd);
    syslog(LOG_DEBUG, "write_pid: succeeded");
    return 0;
}

void sighup_handler(int /*sig*/)
{
    syslog(LOG_NOTICE, "Reloading config file");
}

void sigterm_handler(int /*sig*/)
{
    syslog(LOG_NOTICE, "Terminating daemon");
    closelog();
    exit(0);
}

int Daemon::daemon_main(const std::filesystem::path &file)
{
    config_file = file;
    // Open log
    openlog("my_daemon", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    syslog(LOG_NOTICE, "Daemon started");

    // Demonization
    if (setsid() == -1)
    {
        syslog(LOG_ERR, "setsid failed");
        return -1;
    }

    if (chdir("/root") == -1)
    {
        syslog(LOG_ERR, "chdir failed");
        return -1;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Write PID file
    int pid = check_pid(PID_FILE.c_str());
    if (pid > 0 && kill(pid, SIGTERM) < 0 && errno != ESRCH)
    {
        syslog(LOG_ERR, "kill failed");
        return -1;
    }

    if (write_pid(PID_FILE.c_str()) == -1)
    {
        syslog(LOG_ERR, "Failed writing PID");
        return -1;
    }

    // Signals handling
    struct sigaction sighup_action{};
    sighup_action.sa_handler = sighup_handler;

    struct sigaction sigterm_action{};
    sigterm_action.sa_handler = sigterm_handler;

    if (sigaction(SIGHUP, &sighup_action, nullptr) == -1 || sigaction(SIGTERM, &sigterm_action, nullptr) == -1)
    {
        syslog(LOG_ERR, "sigaction failed");
        return -1;
    }

    // Read config
    std::ifstream f(config_file);
    if (!f.is_open())
    {
        syslog(LOG_ERR, "Failed reading config");
        return -1;
    }

    std::vector<std::string> path_list;
    std::string line;
    while (std::getline(f, line))
    {
        if (!line.empty() && line.back() == '\n')
            line.pop_back();

        path_list.push_back(line);
        syslog(LOG_DEBUG, "Retrieved path: '%s'", line.c_str());
    }

    // Start monitor
    int ret = monitor::start_monitor(path_list);
    if (ret == -1)
    {
        syslog(LOG_DEBUG, "start_monitor failed");
        return -1;
    }

    // Waiting for signals
    sigset_t sig_set;
    if (sigemptyset(&sig_set) == -1 || sigaddset(&sig_set, SIGKILL) == -1)
    {
        syslog(LOG_ERR, "sigset failed");
        return -1;
    }

    int sig;
    return sigwait(&sig_set, &sig);
}


