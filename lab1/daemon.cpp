#include "daemon.h"

std::atomic<bool> Daemon::stopDaemon(false);
std::mutex Daemon::logMutex;
std::atomic<bool> Daemon::readConfig(false);
std::mutex Daemon::configMutex;
std::queue<std::string> Daemon::logQueue;

void Daemon::Start()
{
    pid_t pid;
    try {
        std::string relativePath = "settings.cfg";
        configPath = std::filesystem::absolute(relativePath).string();
        logFile = "size.log";

        pid = fork();
        if (pid < 0) {
            std::stringstream ss;
            ss << "ОШИБКА fork(), (код ошибки errno = " << errno << ")\n"
               <<  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n";
            throw std::runtime_error(ss.str());
        }
        if (pid > 0) {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                      "Завершение работы родительского процесса!\n");
            exit(EXIT_SUCCESS);
        }

        ConnectSignals();

        umask(0);
        pid_t sid = setsid();
        if (sid < 0) {
            std::stringstream ss;
            ss << "ОШИБКА setsid(), (код ошибки errno = " << errno << ")\n"
               <<  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n";
            throw std::runtime_error(ss.str());
        }

        if (chdir("/") < 0) {
            std::stringstream ss;
            ss << "ОШИБКА chdir(), (код ошибки errno = " << errno << ")\n"
               <<  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n";
            throw std::runtime_error(ss.str());
        }

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    catch (const std::exception &ex) {
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_CRIT,
                                                  "ИСКЛЮЧЕНИЕ: %s", ex.what());
        if(pid == 0) {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                      "Завершение работы Daemon!\n");
        }
        else {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                      "Завершение работы родительского процесса!\n");
        }
        exit(EXIT_FAILURE);
    }

    std::filesystem::path currentPath = std::filesystem::current_path();
    Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                              "Старт работы Daemon!\n");
    Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                              "Текущая директория: %s\n", currentPath.string().c_str());
}

void Daemon::ReadConfig()
{
    libconfig::Config cfg;
    std::lock_guard<std::mutex> lock(configMutex);
    try {

        cfg.readFile(configPath.c_str());

        const libconfig::Setting& root = cfg.getRoot();
        const libconfig::Setting& settings = root["settings"];
        const libconfig::Setting& folders = settings["folders"];

        auto count = folders.getLength();
        for (int i = 0; i < count; ++i) {

            const libconfig::Setting& folder = folders[i];

            auto path = getSettingValue<std::string>(folder, "path");
            checkDirectoryExists(path);

            auto number = getSettingValue<int>(folder, "number");
            switch(number) {
            case 1:
                this->folders.first = path;
                break;
            case 2:
                this->folders.second = path;
                break;
            default:
                std::stringstream ss;
                ss << "Задан неверный номер директории: " << path << ".\n"
                   <<  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n";
                throw std::runtime_error(ss.str());
                break;
            }

            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                      "Путь к папке под номером %d: %s\n", number, path.c_str());
        }

        auto time = getSettingValue<int>(settings, "time");
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                  "Интервал: %d секунд\n", time);
        this->time = time;
    }
    catch (const std::exception& ex) {
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_CRIT,
                                                  "ИСКЛЮЧЕНИЕ: %s", ex.what());
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                  "Завершение работы Daemon!\n");
        exit(EXIT_FAILURE);
    }
}

void Daemon::Proccessing()
{
    std::filesystem::path fullPath = std::filesystem::path(folders.second) / logFile;
    while(true) {
        try{
            auto pathSize = getFolderSize(folders.first);
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                      "Размер папки под номером один: %d\n", pathSize);
            Logger::Logger::InstancePtr()->logToFile(fullPath, Logger::LogLevel::_LOG_INFO,
                                                     "Размер папки под номером один: %d\n", pathSize);
            clearFolder(folders.first);
        }
        catch(const std::exception& ex) {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_ERR,
                                                      "ИСКЛЮЧЕНИЕ: %s", ex.what());
            Logger::Logger::InstancePtr()->logToFile(fullPath, Logger::LogLevel::_LOG_ERR,
                                                     "ИСКЛЮЧЕНИЕ: %s", ex.what());
        }

        sleep(time);
    }
}

void Daemon::createPidFile()
{
    int pidFileHandle = open(PID_FILE, O_RDWR | O_CREAT, 0600);
    if (pidFileHandle == -1) {
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_CRIT,
                                                  "Не удалось открыть PID файл %s: %s\n"
                                                  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n", PID_FILE, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (lockf(pidFileHandle, F_TLOCK, 0) == -1) {
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_CRIT,
                                                  "Не удалось заблокировать PID файл %s: %s\n"
                                                  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n", PID_FILE, strerror(errno));
        close(pidFileHandle);
        exit(EXIT_FAILURE);
    }

    char oldPidStr[PID_STR_SIZE] = {0};
    if (read(pidFileHandle, oldPidStr, sizeof(oldPidStr) - 1) > 0) {
        int old_pid = atoi(oldPidStr);

        if (old_pid > 0 && kill(old_pid, 0) == 0) {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                      "Процесс с PID %d уже запущен, отправляем SIGTERM", old_pid);
            kill(old_pid, SIGTERM);
            sleep(1);
        }
    }

    if (ftruncate(pidFileHandle, 0) == -1) {
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_CRIT,
                                                  "Не удалось очистить PID файл %s: %s", PID_FILE, strerror(errno));
        close(pidFileHandle);
        exit(EXIT_FAILURE);
    }

    lseek(pidFileHandle, 0, SEEK_SET);

    std::string pidStr = std::to_string(getpid()) + "\n";
    if (write(pidFileHandle, pidStr.c_str(), pidStr.size()) == -1) {
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_CRIT,
                                                  "Не удалось записать PID в файл %s: %s", PID_FILE, strerror(errno));
        close(pidFileHandle);
        exit(EXIT_FAILURE);
    }

    Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                              "PID файл %s успешно создан с PID %d", PID_FILE, getpid());

    close(pidFileHandle);
}

uintmax_t Daemon::getFolderSize(const std::filesystem::__cxx11::path &folderPath)
{
    uintmax_t totalSize = 0;
    if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                totalSize += std::filesystem::file_size(entry.path());
            }
        }
    } else {
        std::stringstream ss;
        ss << "Указанный путь не существует или это не директория: "
           << folderPath << ".\n";
        throw std::runtime_error(ss.str());
    }

    return totalSize;
}

void Daemon::clearFolder(const std::filesystem::__cxx11::path &folderPath)
{
    if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (std::filesystem::is_directory(entry)) {
                clearFolder(entry.path());
            } else {
                std::filesystem::remove(entry);
            }
        }
    } else {
        std::stringstream ss;
        ss << "Указанный путь не существует или это не директория: "
           << folderPath << ".\n";
        throw std::runtime_error(ss.str());
    }
}

void Daemon::checkDirectoryExists(const std::string &dirPath)
{
    namespace fs = std::filesystem;

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        throw std::runtime_error("Директория \"" + dirPath + "\" не существует.\n");
    }
}

void Daemon::ConnectSignals()
{
    std::memset(&term_handler, 0, sizeof(term_handler));
    term_handler.sa_sigaction = Daemon::termHandler;
    term_handler.sa_flags = SA_RESTART | SA_SIGINFO;

    if (sigaction(SIGTERM, &term_handler, nullptr) == -1) {
        std::stringstream ss;
        ss << "ОШИБКА sigaction(), (код ошибки errno = " << errno << ")\n"
           <<  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n";
        throw std::runtime_error(ss.str());
    }

    std::thread logThread(&Daemon::termLog, this);
    logThread.detach();

    std::memset(&hup_handler, 0, sizeof(hup_handler));
    hup_handler.sa_sigaction = Daemon::hupHandler;
    hup_handler.sa_flags = SA_RESTART | SA_SIGINFO;

    if(sigaction(SIGHUP, &hup_handler, nullptr) == -1) {
        std::stringstream ss;
        ss << "ОШИБКА sigaction(), (код ошибки errno = " << errno << ")\n"
           <<  LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n";
        throw std::runtime_error(ss.str());
    }

    std::thread configThread(&Daemon::hupReadConfig, this);
    configThread.detach();
}

void Daemon::termLog()
{
    while(true) {
        std::lock_guard<std::mutex> lock(logMutex);
        while (!logQueue.empty()) {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::_LOG_INFO,
                                                      logQueue.front().c_str());
            logQueue.pop();

            if (stopDaemon) {
                exit(EXIT_FAILURE);
            }
        }
        if (stopDaemon) {
            exit(EXIT_FAILURE);
        }
    }
}

void Daemon::hupReadConfig()
{
    while(true) {
        if(readConfig == true) {
            ReadConfig();
            readConfig = false;
        }
    }
}

void Daemon::termHandler(int signum, siginfo_t *info, void *ctx)
{
    std::string termMessage = static_cast<std::string>("Завершение работы Daemon! ") + static_cast<std::string>("Получил сигнал ") + std::to_string(signum) +
            " from process " + std::to_string(info->si_pid) + "\n";
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logQueue.push(termMessage);
    }
    stopDaemon = true;
}

template<typename TypeValue>
TypeValue Daemon::getSettingValue(const libconfig::Setting &setting, const std::string &key)
{
    TypeValue value;
    if (!setting.lookupValue(key, value)) {
        std::stringstream ss;
        ss << "Ключ " << key << " не найден в настройках.\n"
           << LOG_INDENT "ПРОДОЛЖЕНИЕ РАБОТЫ НЕ ВОЗМОЖНО\n";
        throw std::runtime_error(ss.str());
    }
    return value;

}
