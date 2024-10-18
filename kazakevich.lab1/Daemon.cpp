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
		checkdir();
		sleep(static_cast<unsigned int>(std::chrono::seconds(configManager->get().interval).count())); 
	}

	pidfileManager->remove();
	syslog(LOG_NOTICE, "Daemon terminated.");
	closelog();
}

void Daemon::handleSignal(int signum)
{
	if (signum == SIGHUP) {
		syslog(LOG_NOTICE, "SIGHUP: reloading the configuration.");
		ConfigManager::getInstance()->loadConfig();
	}
	else if (signum == SIGTERM) {
		syslog(LOG_NOTICE, "SIGTERM: completion of work.");
		Daemon::getInstance()->running = false;
	}
	else {
		syslog(LOG_WARNING, "Unknown signal: %d", signum);
	}
}

void Daemon::checkdir() const
{
	try {
		fs::current_path(configManager->get().workingdirPath);
		syslog(LOG_NOTICE, "The working directory has been changed to: %s", fs::current_path().c_str());
	}
	catch (const fs::filesystem_error& e) {
		syslog(LOG_WARNING, "Error changing the working directory: %s", e.what());
		return;
	}

	auto now = std::chrono::system_clock::now();

	for (const auto& entry : fs::directory_iterator(fs::current_path())) {
		if (fs::is_regular_file(entry.status())) {
			auto lastWriteTime = fs::last_write_time(entry.path());
			auto lastWriteTimeSys = decltype(now)(lastWriteTime.time_since_epoch());

			auto fileAge = std::chrono::duration_cast<std::chrono::minutes>(now - lastWriteTimeSys).count();
			if (fileAge > 1) {
				//std::cerr << "delete file" << std::endl;
				syslog(LOG_INFO, "Deleting file: %s (age: %ld minuts)", entry.path().c_str(), fileAge);
				fs::remove(entry.path());
			}
		}
	}
}
