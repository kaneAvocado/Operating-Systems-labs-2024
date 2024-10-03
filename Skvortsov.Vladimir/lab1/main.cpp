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

  Daemon::get_instance().run(argv[1]);

  return EXIT_SUCCESS;
};
