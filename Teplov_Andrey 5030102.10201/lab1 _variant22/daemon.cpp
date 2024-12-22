#include "daemon.hpp"

// Constants
const char* PID_FILE = "/var/run/os-lab-daemon.pid";
const mode_t DIR_PERMISSIONS = 0777;
const int PID_STR_SIZE = 10;

void Daemon::run(const std::string& config_path, int interval = 5) {
  // Store the current working directory
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    syslog(
      LOG_ERR,
      "Could not get current working directory: %s",
      strerror(errno)
    );
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
  pid_t pid = fork();
  // Handle error
  if (pid < 0) {
    syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  // Parent process. Exit
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  std::cout << "pid: " << pid << std::endl;

  umask(0);

  pid_t sid = setsid();
  if (sid < 0) {
    syslog(LOG_ERR, "setsid failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  std::cout << "sid: " << sid << std::endl;

  if (chdir("/") < 0) {
    syslog(LOG_ERR, "chdir failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  std::cout << "chdir done" << std::endl;

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  create_pid_file();
};

void Daemon::create_pid_file() {
  int pid_file_handle = open(PID_FILE, O_RDWR | O_CREAT, 0600);
  if (pid_file_handle == -1) {
    syslog(
      LOG_ERR,
      "Could not open PID file %s: %s",
      PID_FILE,
      strerror(errno)
    );
    exit(EXIT_FAILURE);
  }

  if (lockf(pid_file_handle, F_TLOCK, 0) == -1) {
    syslog(
      LOG_ERR,
      "Could not lock PID file %s: %s",
      PID_FILE,
      strerror(errno)
    );
    exit(EXIT_FAILURE);
  }

  char old_pid_str[PID_STR_SIZE];
  if (read(pid_file_handle, old_pid_str, sizeof(old_pid_str) - 1) > 0) {
    int old_pid = atoi(old_pid_str);

    if (old_pid > 0 && kill(old_pid, 0) == 0) {
      syslog(
        LOG_INFO,
        "Process with PID %d is already running, sending SIGTERM",
        old_pid
      );
      kill(old_pid, SIGTERM);
      sleep(1);
    }
  }

  ftruncate(pid_file_handle, 0);
  lseek(pid_file_handle, 0, SEEK_SET);

  char str[PID_STR_SIZE];
  sprintf(str, "%d\n", getpid());
  write(pid_file_handle, str, strlen(str));

  syslog(
    LOG_INFO,
    "PID file %s created successfully with PID %d",
    PID_FILE,
    getpid()
  );

  close(pid_file_handle);
};

void Daemon::read_config() {
  std::ifstream config_file(resolve_path(config_path));

  if (!config_file.is_open()) {
    syslog(
      LOG_ERR,
      "Could not open config file %s: %s",
      config_path.c_str(),
      strerror(errno)
    );
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
      folder1 = remove_quotes(folder1);
      folder2 = remove_quotes(folder2);
      subfolder = remove_quotes(subfolder);

      // Resolve relative paths
      folder1 = resolve_path(folder1);
      folder2 = resolve_path(folder2);

      config[folder1] = {folder2, {ext, subfolder}};
    }
  }
  config_file.close();
};

std::string Daemon::remove_quotes(const std::string& str) {
  if (str.front() == '"' && str.back() == '"') {
    return str.substr(1, str.size() - 2);
  }
  return str;
};

std::string Daemon::resolve_path(const std::string &path) {
  if (path.front() != '/') {
    return current_dir + "/" + path;
  }
  return path;
};

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
};

void Daemon::setup_signal_handlers() {
  signal(SIGHUP, signal_handler);
  signal(SIGTERM, signal_handler);
};

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
};

void Daemon::process_folders(
  const std::string& folder1,
  const std::string& folder2,
  const std::string& ext,
  const std::string& subfolder
) {
  DIR* dir = opendir(folder1.c_str());
  if (dir == NULL) {
    syslog(
      LOG_ERR,
      "Could not open directory %s: %s",
      folder1.c_str(),
      strerror(errno)
    );
    return;
  }

  struct dirent* ent;

  const std::string targetSubfolder = folder2 + "/" + subfolder;
  const std::string targetOthers = folder2 + "/OTHERS";

  mkdir(targetSubfolder.c_str(), DIR_PERMISSIONS);
  mkdir(targetOthers.c_str(), DIR_PERMISSIONS);

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
