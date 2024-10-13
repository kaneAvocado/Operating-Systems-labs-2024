#include "daemon.hpp"


void signal_handler(int sig)
{
    switch (sig)
    {
    case SIGHUP:
        Daemon::get_instance().got_sighup = 1;
        break;
    case SIGTERM:
        Daemon::get_instance().got_sigterm = 1;
        break;
    }
}

// Функция для вычисления MD5
std::string calculate_md5(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file: " + filepath);

    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)))
        MD5_Update(&md5Context, buffer, file.gcount());

    // Обрабатываем остаток данных
    MD5_Update(&md5Context, buffer, file.gcount());

    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_Final(result, &md5Context);

    std::ostringstream oss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];

    return oss.str();
}

// Создание файла с MD5 суммами
void Daemon::create_md5_file(const std::filesystem::path& dir_path)
{
    std::string md5_filename = "md5_sums.txt";
    std::filesystem::path md5_filepath = dir_path / md5_filename;

    std::ofstream md5_file(md5_filepath);
    if (!md5_file)
    {
        syslog(LOG_ERR, "Failed to create MD5 file: %s", md5_filepath.c_str());
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(dir_path))
    {
        if (entry.is_regular_file() && entry.path().filename() != md5_filename)
        {
            std::string md5_hash;
            try
            {
                md5_hash = calculate_md5(entry.path().string());
            }
            catch (const std::exception &e)
            {
                syslog(LOG_ERR, "Failed to calculate MD5 for file %s: %s", entry.path().c_str(), e.what());
                continue;
            }

            md5_file << entry.path().filename().string() << " " << md5_hash << std::endl;
        }
    }

    md5_file.close();
    syslog(LOG_INFO, "MD5 sums file created successfully at %s", md5_filepath.c_str());
}

// Чтение конфигурационного файла
void Daemon::open_config_file()
{
    config.read();
}

// Создание PID файла
void Daemon::create_pid_file()
{
    std::string pid_file = "/var/run/daemon.pid";
    int pid_file_handle = open(pid_file.c_str(), O_RDWR | O_CREAT, 0600);
    if (pid_file_handle == -1)
    {
        syslog(LOG_ERR, "PID file %s cannot be opened", pid_file.c_str());
        exit(EXIT_FAILURE);
    }

    if (lockf(pid_file_handle, F_TLOCK, 0) == -1)
    {
        syslog(LOG_ERR, "Daemon is already running (PID file is locked)");
        exit(EXIT_FAILURE);
    }

    ftruncate(pid_file_handle, 0);
    lseek(pid_file_handle, 0, SEEK_SET);

    char str[10];
    snprintf(str, sizeof(str), "%d\n", getpid());
    write(pid_file_handle, str, strlen(str));

    syslog(LOG_INFO, "PID file %s created successfully with PID %d", pid_file.c_str(), getpid());

    close(pid_file_handle);
}


void Daemon::daemonize()
{
    pid_t pid, sid;
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    umask(0);
    sid = setsid();
    if (sid < 0)
        exit(EXIT_FAILURE);
    if ((chdir("/")) < 0)
        exit(EXIT_FAILURE);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    create_pid_file();
}


void Daemon::run(const std::filesystem::path & current_dir, const std::string & filename)
{
    current_path = current_dir;
    config = {filename};

    daemonize();  

    open_config_file();  

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);

    while (true)
    {
        if (got_sighup == 1)
        {
            got_sighup = 0;
            syslog(LOG_INFO, "Re-reading config file");
            open_config_file();  
        }

        if (got_sigterm == 1)
        {
            got_sigterm = 0;
            syslog(LOG_INFO, "Daemon is exiting");
            closelog();
            exit(EXIT_SUCCESS);
        }

        try
        {
            create_md5_file(config.get_directory());
        }
        catch (const std::exception &e)
        {
            syslog(LOG_ERR, "Error while creating MD5 file: %s", e.what());
        }

        std::this_thread::sleep_for(std::chrono::seconds(config.get_interval()));
    }
}
