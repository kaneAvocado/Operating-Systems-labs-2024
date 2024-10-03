#include <csignal>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>

class Daemon {
  public:
    static Daemon& get_instance() {
      static Daemon instance;
      return instance;
    };

    void run(const std::string& config_path, int interval);

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
    std::string remove_quotes(const std::string& str);
    std::string resolve_path(const std::string& path);

    void process_folders(
      const std::string& folder1,
      const std::string& folder2,
      const std::string& ext,
      const std::string& subfolder
    );
};
