#include "Daemon.h"

// public method:
Daemon& Daemon::getInstanceDaemon()
{
	static Daemon d_instance;
	return d_instance;
}


void signalHandler(int signal)
{
	switch (signal)
	{
	case SIGHUP:
		Daemon::getInstanceDaemon().forSIGHUP();
		break;
	case SIGTERM:
		Daemon::getInstanceDaemon().forSIGTERM();
		break;
	default:
		break;
	}
}


void Daemon::initDaemon(const std::string& config_path)
{
	openlog("LOG DAEMON LAB-1.6", LOG_NDELAY | LOG_PID, LOG_USER);
	syslog(LOG_INFO, "Initialize Daemon...");

	// open config file
	d_configAbsolutePath = std::filesystem::absolute(config_path);
	d_config = configFile();

	if (d_config.readConfigFile(d_configAbsolutePath))
	{
		syslog(LOG_INFO, "Config file read succesfully.");
	}
	else
	{
		syslog(LOG_ERR, "Failed read config file.");
	}

	int pid = 0;

	// checking for the existence of a pid file
	bool check_pid = false;
	std::ifstream file(d_pidPath);
	if (file.is_open()) {
		if (file >> pid && kill(pid, 0) == 0) {
			check_pid = true;
		}
		file.close();
	}

	if (check_pid) {
		syslog(LOG_INFO, "Daemon's started! Killing old process...");
		kill(pid, SIGTERM);
	}
	else
		syslog(LOG_INFO, "Daemon hasn't been started.");


	// demonizing the process
	daemonize();


	syslog(LOG_INFO, "Updating pid-file...");
	bool update_pid = false;
	std::ofstream fileO(d_pidPath);

	if (fileO.is_open()) {
		fileO << getpid();
		fileO.close();
		update_pid = true;
	}
	if (!update_pid) {
		syslog(LOG_ERR, "Cannot open pid-file.");
		exit(EXIT_FAILURE);
	}
	else
		syslog(LOG_INFO, "Pid-file update successfully.");


	syslog(LOG_INFO, "Set signal handlers");
	std::signal(SIGHUP, signalHandler);
	std::signal(SIGTERM, signalHandler);

	syslog(LOG_INFO, "Initialize Daemon complete");
}


void Daemon::runDaemon()
{
	syslog(LOG_INFO, "Run Daemon...");
	while (!d_sigTerminate)
	{
		commandDaemon();
		sleep(d_config.INTERVAL_IN_SECONDS);
	}
}



// private method:
void Daemon::daemonize()
{
	int stdin_copy = dup(STDIN_FILENO);
	int stdout_copy = dup(STDOUT_FILENO);
	int stderr_copy = dup(STDERR_FILENO);


	pid_t m_pid = fork();

	if (m_pid > 0) {
		std::exit(EXIT_SUCCESS);
	}
	else if (m_pid < 0) {
		std::exit(EXIT_FAILURE);
	}

	// umask needs to be set so files and logs can be written
	umask(0);
	pid_t m_sid = setsid();

	if (m_sid < 0) 
	{
		syslog(LOG_ERR, "Could not set SID to child process");
		std::exit(EXIT_FAILURE);
	}
	
	// Change the current working directory to a root directory
	if (chdir("/") < 0)
	{
		syslog(LOG_ERR, "Could not change current working directory to root");
		std::exit(EXIT_FAILURE);
	}

	// closs terminal stdin, stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	dup2(stdin_copy, STDIN_FILENO);
	dup2(stdout_copy, STDOUT_FILENO);
	dup2(stderr_copy, STDERR_FILENO);
}


void Daemon::commandDaemon()
{
	syslog(LOG_INFO, "run command Daemon...");

	
	if (!(std::filesystem::exists(d_config.folder1) && std::filesystem::is_directory(d_config.folder1))) {
		syslog(LOG_WARNING, "folder1: does not exist.");
		return;
	}

	if (!(std::filesystem::exists(d_config.folder2) && std::filesystem::is_directory(d_config.folder2))) {
		syslog(LOG_WARNING, "folder2: does not exist.");
		return;
	}
	
	
	syslog(LOG_INFO, "all folders exist.");

	for (const auto& entry : std::filesystem::directory_iterator(d_config.folder1))
	{
		// std::cout << entry.path() << std::endl;
		std::filesystem::file_time_type ftime = std::filesystem::last_write_time(entry.path());

		const auto toNow = std::filesystem::file_time_type::clock::now() - ftime;
		const auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(toNow).count();
		
		if (elapsedSec > d_config.MAX_FILE_AGE_IN_SECONDS)
		{
			syslog(LOG_INFO, "Moving the file from folder1 to folder2...");
			// if a file with the same name has already been moved, it is replaced with its new version
			std::error_code errorCodecopy;
			std::filesystem::copy(entry.path(), d_config.folder2, std::filesystem::copy_options::update_existing, errorCodecopy);
			if (errorCodecopy)
			{
				syslog(LOG_WARNING, "error in copy file from folder1 to folder1");
				continue;
			}

			std::error_code errorCode;
			if (!std::filesystem::remove(entry.path(), errorCode)) {
				syslog(LOG_WARNING, "error in remove file from folder1");
			}
			else
			{
				syslog(LOG_INFO, "Moving file successfully");
			}
			
		}
	}
	syslog(LOG_INFO, "Command Daemon complete");
}


void Daemon::forSIGHUP()
{
	syslog(LOG_INFO, "Config file Re-read");

	if (d_config.readConfigFile(d_configAbsolutePath)) 
	{
		syslog(LOG_INFO, "Config file Re-readed succesfully.");
	}
	else
	{
		syslog(LOG_ERR, "Failed Re-read config file.");
	}
}


void Daemon::forSIGTERM()
{
	syslog(LOG_INFO, "Terminate process...");
	d_sigTerminate = true;
	syslog(LOG_INFO, "Terminated successfully.");
	closelog();
}

