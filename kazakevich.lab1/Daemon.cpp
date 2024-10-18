#include "Daemon.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <syslog.h>
#include <filesystem>

namespace fs = std::filesystem;

Daemon* Daemon::instance_ptr = nullptr;

Daemon* Daemon::getInstance()
{
	if (!instance_ptr) {
		instance_ptr = new Daemon();
	}
	return instance_ptr;
}

void Daemon::run()
{
	configManager = ConfigManager::getInstance();
	pidfileManager = PidfileManager::getInstance();

	openlog("daemon", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Daemon started.");

	if (!configManager->loadConfig()) {
		exit(EXIT_FAILURE);
	}


	while (running) {
		//std::cerr << "check dir" << std::endl;
		checkdir();
		//std::cerr << "sleep" << std::endl;
		sleep(static_cast<unsigned int>(std::chrono::seconds(configManager->get().interval).count())); //миллисекунды в секунды
	}

	// Очистка и завершение
	pidfileManager->remove();
	syslog(LOG_NOTICE, "Daemon terminated.");
	closelog();
	//std::cerr << "deamon terminated" << std::endl;
}

void Daemon::handleSignal(int signum)
{
	if (signum == SIGHUP) {
		syslog(LOG_NOTICE, "SIGHUP: перезагрузка конфигурации.");
		ConfigManager::getInstance()->loadConfig();
	}
	else if (signum == SIGTERM) {
		syslog(LOG_NOTICE, "SIGTERM: завершение работы.");
		Daemon::getInstance()->running = false;
	}
	else {
		syslog(LOG_WARNING, "Неизвестный сигнал: %d", signum);
	}
}

void Daemon::checkdir() const
{
	//смена рабочей дирректории
	try {
		fs::current_path(configManager->get().workingdirPath);
		syslog(LOG_NOTICE, "Рабочая директория изменена на: %s", fs::current_path().c_str());
		//std::cerr << "Рабочая директория изменена на: " << fs::current_path().c_str();
	}
	catch (const fs::filesystem_error& e) {
		syslog(LOG_WARNING, "Ошибка изменения рабочей директории: %s", e.what());
		//std::cerr << "Ошибка изменения рабочей директории" << std::endl;
		return;
	}

	// проверять все файлы по возрасту и удалять старые
	// Текущий момент времени
	auto now = std::chrono::system_clock::now();

	// Проходим по всем файлам в директории
	for (const auto& entry : fs::directory_iterator(fs::current_path())) {
		// Проверяем, является ли текущий элемент файлом
		if (fs::is_regular_file(entry.status())) {
			auto lastWriteTime = fs::last_write_time(entry.path());
			auto lastWriteTimeSys = decltype(now)(lastWriteTime.time_since_epoch());

			// Проверяем, прошло ли более 1 минуты с момента последнего изменения файла
			auto fileAge = std::chrono::duration_cast<std::chrono::minutes>(now - lastWriteTimeSys).count();
			if (fileAge > 1) {
				//std::cerr << "delete file" << std::endl;
				syslog(LOG_INFO, "Удаление файла: %s (возраст: %ld минут)", entry.path().c_str(), fileAge);
				fs::remove(entry.path());
			}
		}
	}
}
