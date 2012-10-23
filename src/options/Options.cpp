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
long Options::HEARTBEAT_TIMEOUT_MILLIS;
std::vector<std::string> Options::MULTI_STAT_SERVICES;
std::vector<std::string> Options::LONGLONG_SERVICES;

bool Options::IS_MERGER;

void Options::PrintVM(po::variables_map vm) {
	using namespace po;
	for (variables_map::iterator it = vm.begin(); it != vm.end(); ++it) {
		mycout << it->first << "=";

		const variable_value& v = it->second;
		if (!v.empty()) {
			const std::type_info& type = v.value().type();
			if (type == typeid(::std::string)) {
				mycout << v.as<std::string>();
			} else if (type == typeid(int)) {
				mycout << v.as<int>();
			}
		}
		mycout << std::endl;
	}
}
/**
 * The constructor must be public but should not be called! Use Instance() as factory Method instead.
 */
void Options::Initialize(int argc, char* argv[]) {
	po::options_description desc("Allowed options");
	desc.add_options()(OPTION_HELP, "Produce help message")(OPTION_VERBOSE,
			"Verbose mode")(OPTION_CONFIG_FILE,
			po::value<std::string>()->default_value("/etc/na62-farm-dim.conf"),
			"Config file for these options")(OPTION_FARM_EXEC_Path,
			po::value<std::string>()->required(),
			"Path to the executable farm program")(
			OPTION_HEARTBEAT_TIMEOUT_MILLIS, po::value<int>()->required(),
			"Number of milliseconds that have to pass without receiving a heart beat from the farm program until we go into error mode.")(
			OPTION_MULTI_STAT_SERVICES, po::value<std::string>()->required(),
			"Comma separated (S1,S2,S3...) list of services with multiple stats like \"A:a;B:b\"")(
			OPTION_LONGLONG_SERVICES, po::value<std::string>()->required(),
			"Comma separated list (S1,S2,S3...) of services with single long values.")
			(OPTION_IS_MERGER, po::value<int>()->required(), "Set 1 if this is the dim connector for a merger and 0 if it is running on a farm PC.");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count(OPTION_HELP)) {
		mycout << desc << "\n";
		exit(EXIT_SUCCESS);
	}

	mycout << "=======Reading config file "
			<< vm[OPTION_CONFIG_FILE ].as<std::string>() << std::endl;
	po::store(
			po::parse_config_file<char>(
					vm[OPTION_CONFIG_FILE ].as<std::string>().data(), desc),
			vm);
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm); // Check the configuration

	mycout << "=======Running with following configuration:" << std::endl;
	PrintVM(vm);

	VERBOSE = vm.count(OPTION_VERBOSE) > 0;

	FARM_EXEC_PATH = vm[OPTION_FARM_EXEC_Path ].as<std::string>();
	if (!boost::filesystem::exists(FARM_EXEC_PATH)) {
		throw BadOption(OPTION_FARM_EXEC_Path, "File does not exist!");
	}

	HEARTBEAT_TIMEOUT_MILLIS = vm[OPTION_HEARTBEAT_TIMEOUT_MILLIS ].as<int>();

	boost::split(MULTI_STAT_SERVICES,
			vm[OPTION_MULTI_STAT_SERVICES ].as<std::string>(),
			boost::is_any_of(","));
	boost::split(LONGLONG_SERVICES,
			vm[OPTION_LONGLONG_SERVICES ].as<std::string>(),
			boost::is_any_of(","));

	IS_MERGER=vm[OPTION_IS_MERGER].as<int>()>0;
}
} /* namespace dim */
} /* namespace na62 */
