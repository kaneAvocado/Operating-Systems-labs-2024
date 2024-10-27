#pragma once
#include <memory>
#include <string>
#include <sys/types.h>


class PidfileManager
{
public:
	static PidfileManager* getInstance();

	bool create();
	void remove() const;

	void setPidFilePath(std::string path);

private:
	static PidfileManager* instance_ptr;
	std::string pidFilePath;

	bool processExists(pid_t pid) const;
	bool isExistingPidRunning() const;
	bool terminateExistingProcess() const;
	bool writePidToFile() const;

	PidfileManager() = default;
	PidfileManager(const PidfileManager&) = delete;
	~PidfileManager() { delete instance_ptr; }
};

