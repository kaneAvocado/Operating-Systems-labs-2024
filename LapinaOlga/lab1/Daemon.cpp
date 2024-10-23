#include "Daemon.h"


void Daemon::signalHandler(int signal)
{
	switch (signal)
	{
	case SIGHUP:
		Daemon::getInstanceDaemon().forSIGHUP();
	case SIGTERM:
		Daemon::getInstanceDaemon().forSIGTERM();
	}
}

void Daemon::forSIGHUP()
{
	syslog(LOG_INFO, "Config file reload");
	if (config.parse_config(ABSOLUTE_CONFIG_PATH)) {
		syslog(LOG_INFO, "Config' been reloaded succesfully.");
	}
	else
		syslog(LOG_ERR, "Failed reloading config params.");
}


void Daemon::forSIGTERM()
{
	syslog(LOG_INFO, "Terminate process...");
	sigTerminate = true;
	syslog(LOG_INFO, "Terminated successfully.");
	closelog();
}


void Daemon::runDaemon()
{
	if (!sigTerminate)
	{
		syslog(LOG_INFO, "Run Daemon command...");
		commandDaemon();
		sleep(config.INTERVAL_IN_SECONDS);
	}
}
