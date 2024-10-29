#include "daemon.hpp"
#include <cstring>
#define PID_PATH "/var/run/daemon.pid"
#define MD5_NAME "md5_sums.txt"

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
std::string calculate_md5(const std::string& filename) {
    unsigned char result[EVP_MAX_MD_SIZE];  // Хэш результат
    unsigned int result_len = 0;

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();  // Создаем контекст
    if (mdctx == nullptr) {
        throw std::runtime_error("Failed to create MD5 context.");
    }

    const EVP_MD* md = EVP_md5();  // Используем MD5
    if (!EVP_DigestInit_ex(mdctx, md, nullptr)) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize MD5 digest.");
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to open file: " + filename);
    }

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        if (!EVP_DigestUpdate(mdctx, buffer, file.gcount())) {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to update MD5 digest.");
        }
    }

    if (!EVP_DigestFinal_ex(mdctx, result, &result_len)) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize MD5 digest.");
    }

    EVP_MD_CTX_free(mdctx);  // Освобождаем ресурсы

    // Преобразуем результат в строку с hex представлением
    std::ostringstream oss;
    for (unsigned int i = 0; i < result_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }
    return oss.str();
}

// Создание файла с MD5 суммами
void Daemon::create_md5_file(const std::filesystem::path& dir_path)
{
    std::string md5_filename = MD5_NAME;
    std::filesystem::path md5_filepath = dir_path / md5_filename;

    std::ofstream md5_file(md5_filepath);
    if (!md5_file)
    {
        syslog(LOG_ERR, "Failed to create MD5 file: %s", md5_filepath.c_str());
        exit(EXIT_FAILURE);
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
void Daemon::open_config_file(const std::string &filename)
{
    config.read(filename);
}

// Создание PID файла
bool is_process_running(pid_t pid) {
    // Проверка существования процесса с указанным PID через систему /proc
    return (kill(pid, 0) == 0);
}

// Функция для проверки существующего демона
bool check_existing_daemon(const std::string &pid_file) {
    std::ifstream file(pid_file);
    if (file.is_open()) {
        pid_t existing_pid;
        file >> existing_pid;

        // Проверяем, есть ли процесс с таким PID
        if (existing_pid > 0 && is_process_running(existing_pid)) {
            syslog(LOG_INFO, "Daemon is already running with PID %d", existing_pid);
            return true; // Демон уже запущен
        }
        syslog(LOG_INFO, "PID file exists but no process with PID %d found", existing_pid);
    }
    return false; // Демона нет, можно запускать новый
}

void Daemon::create_pid_file() {
    std::string pid_file = PID_PATH;

    // Проверяем наличие работающего демона
    if (check_existing_daemon(pid_file)) {
        syslog(LOG_ERR, "Daemon is already running. Exiting.");
        exit(EXIT_FAILURE);  // Новый демон не создаётся
    }

    // Открываем/создаём PID файл
    int pid_file_handle = open(pid_file.c_str(), O_RDWR | O_CREAT, 0600);
    if (pid_file_handle == -1) {
        syslog(LOG_ERR, "PID file %s cannot be opened", pid_file.c_str());
        exit(EXIT_FAILURE);
    }

    // Блокируем PID файл, чтобы исключить параллельный запуск другого демона
    if (lockf(pid_file_handle, F_TLOCK, 0) == -1) {
        syslog(LOG_ERR, "Failed to lock PID file %s. Another daemon instance might be running.", pid_file.c_str());
        exit(EXIT_FAILURE);
    }

    // Очищаем содержимое PID файла
    ftruncate(pid_file_handle, 0);
    lseek(pid_file_handle, 0, SEEK_SET);

    // Записываем новый PID в файл
    char str[10];
    snprintf(str, sizeof(str), "%d\n", getpid());
    write(pid_file_handle, str, strlen(str));

    syslog(LOG_INFO, "PID file %s created successfully with PID %d", pid_file.c_str(), getpid());

    // Оставляем файл открытым на время работы демона (для сохранения блокировки)
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
    config = {current_dir};

    daemonize();  

    open_config_file(filename);  

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);

    while (true)
    {
        if (got_sighup == 1)
        {
            got_sighup = 0;
            syslog(LOG_INFO, "Re-reading config file");
            open_config_file(filename);  
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
