#include "Daemon.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <syslog.h>


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
		end_running(EXIT_FAILURE);
	}

	pidfileManager->setPidFilePath(configManager->get().pidfilePath);

	if (!pidfileManager->create()) {
		end_running(EXIT_FAILURE);
	}

	while (running) {
		checkdir();
		sleep(static_cast<unsigned int>(std::chrono::seconds(configManager->get().interval).count())); 
	}

	end_running(-1);
}

void Daemon::handleSignal(int signum)
{
	//std::cerr << "daemon signal " << signum << std::endl;
	if (signum == SIGHUP) {
		//std::cerr << "UP" << std::endl;
		syslog(LOG_NOTICE, "SIGHUP: reload configuration.");
		ConfigManager::getInstance()->loadConfig();
	}
	else if (signum == SIGTERM) {
		//std::cerr << "Term" << std::endl;
		syslog(LOG_NOTICE, "SIGTERM: stop working.");
		Daemon::getInstance()->running = false;
	}
	else {
		//std::cerr << "undefined signal" << std::endl;
		syslog(LOG_WARNING, "Undefined signal: %d", signum);
	}
}

void Daemon::checkdir()
{
	auto now = std::chrono::system_clock::now();
	for (const auto& entry : fs::directory_iterator(configManager->get().workingdirPath)) {
		if (fs::is_regular_file(entry.path())) {
			auto lastWriteTime = fs::last_write_time(entry.path());
			auto lastWriteTimeSys = decltype(now)(lastWriteTime.time_since_epoch());

			auto fileAge = std::chrono::duration_cast<std::chrono::minutes>(now - lastWriteTimeSys).count();
			if (fileAge > 1) {
				syslog(LOG_INFO, "Deleting a file: %s (age: %ld minuts)", entry.path().c_str(), fileAge);
				fs::remove(entry.path());
			}
		}
	}

}

void Daemon::end_running(int num_exit = -1)
{
	pidfileManager->remove();
	syslog(LOG_NOTICE, "Daemon terminated.");
	closelog();

	if (num_exit != -1) {
		exit(num_exit);
	}
}
