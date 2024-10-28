#include "daemon.h"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("Incorrect number of arguments: enter 1 argument - name of the configuration file (*.txt)\n");
    return EXIT_FAILURE;
  }

  Daemon::get_instance().inition(argv[1]);

  Daemon::get_instance().run();

  return EXIT_SUCCESS;
}
