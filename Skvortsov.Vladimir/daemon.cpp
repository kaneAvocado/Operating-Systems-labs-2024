#include "daemon.hpp"
#include <regex>
#include <unistd.h>
#include <limits.h>

void Daemon::run(const std::string& config_path, int interval = 5) {
  // Store the current working directory
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    syslog(LOG_ERR, "Could not get current working directory, exiting");
    exit(EXIT_FAILURE);
  }
  this->current_dir = std::string(cwd);

  this->config_path = config_path;
  this->interval = interval;

  openlog("os-lab-daemon", LOG_PID | LOG_CONS, LOG_DAEMON);
  syslog(LOG_INFO, "Daemon starting up");

  daemonize();
  read_config();
  setup_signal_handlers();
  run_main_loop();
};

void Daemon::daemonize() {
  pid_t pid, sid;

  pid = fork();
  // Handle error
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  // Parent process. Exit
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  std::cout << "pid: " << pid << std::endl;

  umask(0);

  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }
  std::cout << "sid: " << sid << std::endl;

  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }
  std::cout << "chdir done" << std::endl;

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  create_pid_file();
}

void Daemon::create_pid_file() {
  const char* pid_file = "/var/run/os-lab-daemon.pid";
  int pid_file_handle = open(pid_file, O_RDWR | O_CREAT, 0600);

  if (pid_file_handle == -1) {
    syslog(LOG_ERR, "Could not open PID file %s, exiting", pid_file);
    exit(EXIT_FAILURE);
  }

  if (lockf(pid_file_handle, F_TLOCK, 0) == -1) {
    syslog(LOG_ERR, "Could not lock PID file %s, exiting", pid_file);
    exit(EXIT_FAILURE);
  }

  char old_pid_str[10];
  if (read(pid_file_handle, old_pid_str, sizeof(old_pid_str) - 1) > 0) {
    int old_pid = atoi(old_pid_str);

    if (old_pid > 0 && kill(old_pid, 0) == 0) {
      syslog(LOG_INFO, "Process with PID %d is already running, sending SIGTERM", old_pid);
      kill(old_pid, SIGTERM);
      sleep(1);
    } else {
      syslog(LOG_INFO, "No process found with PID %d, continuing...", old_pid);
    }
  }

  ftruncate(pid_file_handle, 0);
  lseek(pid_file_handle, 0, SEEK_SET);

  char str[10];
  sprintf(str, "%d\n", getpid());
  write(pid_file_handle, str, strlen(str));

  syslog(LOG_INFO, "PID file %s created successfully with PID %d", pid_file, getpid());

  close(pid_file_handle);
}

void Daemon::read_config() {
  std::ifstream config_file(current_dir + "/" + config_path);

  if (!config_file.is_open()) {
    syslog(LOG_ERR, "Could not open config file %s", config_path.c_str());
    exit(EXIT_FAILURE);
  }

  std::string line;
  std::regex re(R"((\"[^\"]+\"|\S+)\s+(\"[^\"]+\"|\S+)\s+(\S+)\s+(\"[^\"]+\"|\S+))");
  while (std::getline(config_file, line)) {
    std::smatch match;
    if (std::regex_search(line, match, re) && match.size() == 5) {
      std::string folder1 = match[1].str();
      std::string folder2 = match[2].str();
      std::string ext = match[3].str();
      std::string subfolder = match[4].str();

      // Remove quotes if present
      if (folder1.front() == '"' && folder1.back() == '"') {
        folder1 = folder1.substr(1, folder1.size() - 2);
      }
      if (folder2.front() == '"' && folder2.back() == '"') {
        folder2 = folder2.substr(1, folder2.size() - 2);
      }
      if (subfolder.front() == '"' && subfolder.back() == '"') {
        subfolder = subfolder.substr(1, subfolder.size() - 2);
      }

      // Resolve relative paths
      if (folder1.front() != '/') {
        folder1 = current_dir + "/" + folder1;
      }
      if (folder2.front() != '/') {
        folder2 = current_dir + "/" + folder2;
      }

      config[folder1] = {folder2, {ext, subfolder}};
    }
  }
  config_file.close();
}

void signal_handler(int sig) {
  switch (sig) {
    case SIGHUP:
      Daemon::get_instance().read_config();
      syslog(LOG_INFO, "Re-read config file");
      break;
    case SIGTERM:
      syslog(LOG_INFO, "Received SIGTERM, exiting");
      closelog();
      exit(EXIT_SUCCESS);
      break;
  }
}

void Daemon::setup_signal_handlers() {
  signal(SIGHUP, signal_handler);
  signal(SIGTERM, signal_handler);
}

void Daemon::run_main_loop() {
  while (true) {
    for (const auto& entry : config) {
      const std::string& folder1 = entry.first;
      const std::string& folder2 = entry.second.first;
      const std::string& ext = entry.second.second.first;
      const std::string& subfolder = entry.second.second.second;

      process_folders(folder1, folder2, ext, subfolder);
    }

    sleep(interval);
  }
}

void Daemon::process_folders(
  const std::string& folder1,
  const std::string& folder2,
  const std::string& ext,
  const std::string& subfolder
) {
  DIR* dir = opendir(folder1.c_str());

  if (dir == NULL)
  {
    syslog(LOG_ERR, "Could not open directory %s", folder1.c_str());
    return;
  }

  struct dirent *ent;

  const std::string targetSubfolder = folder2 + "/" + subfolder;
  const std::string targetOthers = folder2 + "/OTHERS";

  mkdir(targetSubfolder.c_str(), 0777);
  mkdir(targetOthers.c_str(), 0777);

  while ((ent = readdir(dir)) != NULL) {
    if (ent->d_type != DT_REG)
      continue;

    const std::string fileName = ent->d_name;
    const std::string fileExt = fileName.substr(fileName.find_last_of(".") + 1);
    const std::string targetFolder = (fileExt == ext) ? targetSubfolder : targetOthers;
    const std::string sourceFile = folder1 + "/" + fileName;
    const std::string targetFile = targetFolder + "/" + fileName;

    if (rename(sourceFile.c_str(), targetFile.c_str()) != 0) {
      syslog(
        LOG_ERR,
        "Error moving file %s to %s: %s",
        sourceFile.c_str(),
        targetFile.c_str(),
        strerror(errno)
      );
    }
  }

  closedir(dir);
};
