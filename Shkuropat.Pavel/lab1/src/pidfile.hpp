// include/pidfile.hpp
#ifndef PIDFILE_HPP
#define PIDFILE_HPP

#include <string>

// Создание PID-файла. Возвращает true при успешном создании, иначе false.
bool createPidFile(const std::string& pidFile);

// Удаление PID-файла.
void removePidFile(const std::string& pidFile);

#endif // PIDFILE_HPP
