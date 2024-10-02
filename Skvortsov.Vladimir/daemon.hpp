#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <csignal>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstring>
#include <errno.h>
#include <regex>
#include <sys/wait.h>

class Daemon {
  public:
    static Daemon& get_instance() {
      static Daemon instance;
      return instance;
    };

    void run(const std::string &config_path, int interval);

  private:
    Daemon() = default;
    ~Daemon() = default;
    Daemon(const Daemon&) = delete;
    Daemon& operator = (const Daemon&) = delete;

    std::string current_dir;
    std::string config_path;

    std::map<std::string, std::pair<std::string, std::pair<std::string, std::string>>> config;
    int interval;

    void daemonize();
    void create_pid_file();
    void read_config();
    void setup_signal_handlers();
    friend void signal_handler(int sig);
    void run_main_loop();

    void process_folders(
      const std::string& folder1,
      const std::string& folder2,
      const std::string& ext,
      const std::string& subfolder
    );
};
