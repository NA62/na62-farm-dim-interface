/*
 * Options.cpp
 *
 *  Created on: Nov 9, 2011
 *      Author: kunzejo
 */

#include<boost/filesystem.hpp>
#include "Options.h"

namespace na62 {
namespace dim {

namespace po = boost::program_options;

/*
 * Configurable Variables
 */
bool Options::VERBOSE;
std::string Options::FARM_EXEC_PATH;

void Options::PrintVM(po::variables_map vm) {
	using namespace po;
	for (variables_map::iterator it = vm.begin(); it != vm.end(); ++it) {
		std::cout << it->first << "=";

		const variable_value& v = it->second;
		if (!v.empty()) {
			const std::type_info& type = v.value().type();
			if (type == typeid(::std::string)) {
				std::cout << v.as<std::string>();
			} else if (type == typeid(int)) {
				std::cout << v.as<int>();
			}
		}
		std::cout << std::endl;
	}
}
/**
 * The constructor must be public but should not be called! Use Instance() as factory Method instead.
 */
void Options::Initialize(int argc, char* argv[]) {
	po::options_description desc("Allowed options");
	desc.add_options()(OPTION_HELP, "Produce help message")(OPTION_VERBOSE, "Verbose mode")(OPTION_CONFIG_FILE,
			po::value<std::string>()->default_value("/etc/na62-farm-dim.conf"), "Config file for these options")
			(OPTION_FARM_EXEC_Path, 	po::value<std::string>()->required(), "Path to the executable farm program");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count(OPTION_HELP)) {
		std::cout << desc << "\n";
		exit(EXIT_SUCCESS);
	}

	std::cout << "=======Reading config file " << vm[OPTION_CONFIG_FILE ].as<std::string>() << std::endl;
	po::store(po::parse_config_file<char>(vm[OPTION_CONFIG_FILE ].as<std::string>().data(), desc), vm);
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm); // Check the configuration

	std::cout << "=======Running with following configuration:" << std::endl;
	PrintVM(vm);

	VERBOSE = vm.count(OPTION_VERBOSE) > 0;

	FARM_EXEC_PATH = vm[OPTION_FARM_EXEC_Path ].as<std::string>();
	if (!boost::filesystem::exists(FARM_EXEC_PATH)) {
		throw BadOption(OPTION_FARM_EXEC_Path, "File does not exist!");
	}
}
} /* namespace dim */
} /* namespace na62 */
