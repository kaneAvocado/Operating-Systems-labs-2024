#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include "daemon.cpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Incorrect parameters. Usage: " << argv[0] << " <config_file_path>" << std::endl;
    return EXIT_FAILURE;
  }

  const char* config_file_path = argv[1];

  if (access(config_file_path, R_OK) != 0) {
    std::cerr << "Error: Cannot access config file " << config_file_path << ": " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  try {
    std::cout << "Starting daemon with config file: " << config_file_path << std::endl;
    Daemon::get_instance().run(config_file_path);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
};
