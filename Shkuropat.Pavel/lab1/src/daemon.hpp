// include/daemon.hpp
#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <string>
#include <vector>
#include <utility>
#include <filesystem>


class Daemon {
public:
    // Получение единственного экземпляра класса (Singleton)
    static Daemon& getInstance();

    // Запуск демона с указанием конфигурационного файла и PID-файла
    void run(const std::string& configFile, const std::string& pidFile, const std::string& current_path);

    // Обработчик сигналов
    static void handleSignal(int signum);

    

private:
    // Конструктор и деструктор
    Daemon();
    ~Daemon();

    // Удаление копий и оператор присваивания (Singleton)
    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;

    // Загрузка конфигурации из файла
    void loadConfig(const std::string& configFile);

    // Выполнение действий по конфигурации
    void performActions();

    // Удаление содержимого папки
    void deleteFolderContents(const std::string& folder);

    // Переменные состояния
    bool running;
    int interval; // Интервал в секундах между действиями
    std::string configPath;
    std::filesystem::path newConfPath;
    std::vector<std::pair<std::string, std::string>> config;
};

#endif // DAEMON_HPP
