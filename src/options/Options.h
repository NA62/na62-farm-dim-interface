/*
 * Options.h
 *
 *  Created on: Nov 9, 2011
 *      Author: kunzejo
 */

#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <iostream>
#include <stdint.h>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

#include "../exceptions/BadOption.h"
#include "../exceptions/UnknownSourceIDFound.h"
#include "../utils/Utils.h"

namespace na62 {
namespace dim {
/*
 * Compile time options
 */
#define MTU 1500
#define LKR_SOURCE_ID 0x24

/*
 * Dynamic Options
 */
#define OPTION_HELP (char*)"help"
#define OPTION_VERBOSE (char*)"verbose"
#define OPTION_CONFIG_FILE (char*)"configFile"

#define OPTION_FARM_EXEC_Path (char*)"farmExecPath"
#define OPTION_HEARTBEAT_TIMEOUT_MILLIS (char*)"hearbeatTimeoutMillis"

#define OPTION_MULTI_STAT_SERVICES (char*)"multiStatServices"
#define OPTION_LONGLONG_SERVICES (char*)"singlelongServices"
#define OPTION_IS_MERGER (char*)"isMerger"

class Options {
public:
	static void PrintVM(boost::program_options::variables_map vm);
	static void Initialize(int argc, char* argv[]);


	/*
	 * Configurable Variables
	 */
	static bool VERBOSE;

	static std::string FARM_EXEC_PATH;

	static long HEARTBEAT_TIMEOUT_MILLIS;

	static std::vector<std::string> MULTI_STAT_SERVICES;
	static std::vector<std::string> LONGLONG_SERVICES;

	static bool IS_MERGER;
};
}/* namespace dim */
}/* namespace na62 */
#endif /* OPTIONS_H_ */
