#pragma once
#include <string>
#include <map>
#include <memory>

struct ConfigParams {
	std::string pidfilePath;
	std::string workingdirPath;
	int interval;
};

class ConfigManager
{
public:
	static ConfigManager* getInstance();

	bool loadConfig();
	ConfigParams get() const;
	void setConfigPath(std::string path);

private:
	static ConfigManager* instance_ptr;
	ConfigManager() = default;
	ConfigManager(const ConfigManager&) = delete;
	~ConfigManager() { delete instance_ptr; }

	std::string configPath = "";
	ConfigParams configParams;

	std::string trim(std::string str);
	std::string resolve_path(const std::string& path);
	std::string current_dir;
};

