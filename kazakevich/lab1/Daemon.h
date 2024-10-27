#pragma once
#include <memory>
#include <string>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <dirent.h>
#include <cstring>
#include <filesystem>
#include <errno.h>

#include "ConfigManager.h"
#include "PidfileManager.h"

class Daemon
{
public:
	static Daemon* getInstance();
	void run();
	static void handleSignal(int signum);

private:
	bool running = true;

	ConfigManager* configManager;
	PidfileManager* pidfileManager;

	void checkdir();
	void end_running(int num_exit);

	static Daemon* instance_ptr;

	Daemon() = default;
	Daemon(const Daemon&) = delete;
	~Daemon() { delete instance_ptr; }
};

