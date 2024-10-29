#include "my_daemon.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path cfg_file = std::filesystem::absolute(argv[1]);
    if (realpath(argv[1], NULL) == nullptr)
    {
        std::cerr << "Config file not found" << std::endl;
        return -1;
    }

    std::cout << "Config file: " << argv[1] << std::endl;

    pid_t daemon_pid = fork();
    if (daemon_pid > 0)
    {
        std::cout << "Daemon PID = " << daemon_pid << std::endl;
    }
    else if (daemon_pid == 0)
    {
        return Daemon::get_instance().daemon_main(cfg_file);
    }
    else
    {
        std::cerr << "Failed starting daemon" << std::endl;
    }

    std::cout << "Parent terminated" << std::endl;
    return 0;
}