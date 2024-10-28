#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

class configFile
{
private:
	std::string parseStringFromConfig(std::string s);
public:

	bool readConfigFile(const std::string& config_path);

	std::string folder1;
	std::string folder2;
	int MAX_FILE_AGE_IN_SECONDS;
	int INTERVAL_IN_SECONDS;

};

#endif
