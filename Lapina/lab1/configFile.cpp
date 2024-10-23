#include "configFile.h"


std::string configFile::parseStringFromConfig(std::string s)
{
	std::stringstream string_config(s);
	std::string segment;
	std::vector<std::string> seglist;
	while (std::getline(string_config, segment, '='))
	{
		seglist.push_back(segment);
	}

	if (seglist.size() != 2)
	{
		return "";
	}

	return seglist[1];
}

bool configFile::readConfigFile(const std::string& config_path)
{
	std::ifstream config_file(config_path);
	if (!config_file.is_open())
	{
		return false;
	}

	std::string s;
	int count = 0;
	while (getline(config_file, s)) 
	{
		std::string res_str = parseStringFromConfig(s);
		switch (count)
		{
		case 0: 
			folder1 = res_str; 
			break;
		case 1: 
			folder2 = res_str; 
			break;
		case 2: 
			MAX_FILE_AGE_IN_SECONDS = atoi(res_str.c_str())*60;
			break;
		case 3: 
			INTERVAL_IN_SECONDS = atoi(res_str.c_str()); 
			break;
		default:
			break;
		}

		count += 1;
	}

	config_file.close();

	if (count != 4)
	{
		return false;
	}

	return true;
}