#include "daemon.h"


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
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                                      "Завершение работы родительского процесса!\n");
            exit(EXIT_SUCCESS);
        }

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
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::CRITICAL,
                                                  "ИСКЛЮЧЕНИЕ: %s", ex.what());
        if(pid == 0) {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                                      "Завершение работы Daemon!\n");
        }
        else {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                                      "Завершение работы родительского процесса!\n");
        }
        exit(EXIT_FAILURE);
    }

    std::filesystem::path currentPath = std::filesystem::current_path();
    Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                              "Старт работы Daemon!\n");
    Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                              "Текущая директория: %s\n", currentPath.string().c_str());
}

void Daemon::ReadConfig()
{
    libconfig::Config cfg;
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

            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                                      "Путь к папке под номером %d: %s\n", number, path.c_str());
        }

        auto time = getSettingValue<int>(settings, "time");
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                                  "Интервал: %d секунд\n", time);
        this->time = time;
    }
    catch (const std::exception& ex) {
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::CRITICAL,
                                                  "ИСКЛЮЧЕНИЕ: %s", ex.what());
        Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
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
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::INFO,
                                                      "Размер папки под номером один: %d\n", pathSize);
            Logger::Logger::InstancePtr()->logToFile(fullPath, Logger::LogLevel::INFO,
                                                     "Размер папки под номером один: %d\n", pathSize);
            clearFolder(folders.first);
        }
        catch(const std::exception& ex) {
            Logger::Logger::InstancePtr()->logMessage(Logger::LogLevel::ERROR,
                                                      "ИСКЛЮЧЕНИЕ: %s", ex.what());
            Logger::Logger::InstancePtr()->logToFile(fullPath, Logger::LogLevel::ERROR,
                                                     "ИСКЛЮЧЕНИЕ: %s", ex.what());
        }

        sleep(time);
    }
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
