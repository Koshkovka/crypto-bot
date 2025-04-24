#include "utils/parseJSON.h"

using json = nlohmann::json;

bool getConfigValue(const std::string& key, std::string& data) {
	std::string config_path = std::filesystem::current_path().parent_path().parent_path().parent_path().string() + "/config.json";

	std::ifstream config_file(config_path);
	if (!config_file.is_open()) {
		std::cerr << "Error: could not open config.json" << std::endl;
		return false;
	}

	json config;
	config_file >> config;

	size_t dot_pos = key.find('.');
	std::string parent_key = key.substr(0, dot_pos);
	std::string child_key = key.substr(dot_pos + 1);

	if (config.contains(parent_key) && config[parent_key].contains(child_key)) {
		data = config[parent_key][child_key];
		return true;
	}
	else if (config.contains(key)) {
		data = config[key];
		return true;
	}
	else {
		std::cerr << "Error: key '" << key << "' not in config.json" << std::endl;
		return false;
	}
}