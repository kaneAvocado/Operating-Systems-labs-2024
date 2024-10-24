#pragma once
#include <string>
#include <fstream>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>
#include <filesystem>
#include <iostream>

struct ConfigParams {
	std::string currentDir;
	std::string workingFolder1, workingFolder2;
	int interval;
};

class Daemon
{
	static Daemon* instance;
	Daemon() = default;
	Daemon(const Daemon&) = delete;
	~Daemon() = default;

	std::string configPath;
	void readConfig(std::string configPath);
	void readConfig();
	std::string complementPath(std::string path);
	std::string trim(const std::string str);

	void demonizeProcess();

	std::string pidfilePath;
	bool createPid(std::string pidfilePath);
	bool createPid();
	void removePid() const;

	ConfigParams configParams;

	void check_working_folders();

	bool end_program = false;
	
public:
	static Daemon* getInstance();
	static void handleSignal(int signum);

	void run(std::string configPath, std::string pidfilePath);
};

