/*
 * FarmStarter.cpp
 *
 *  Created on: Sep 12, 2012
 *      Author: root
 */

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include "options/Options.h"

#include "FarmStarter.h"

namespace na62 {
namespace dim {

FarmStarter::FarmStarter() :
		availableSourceIDs_("RunControl/EnabledDetectors", -1, this), farmPID_(
				-1) {
}

FarmStarter::~FarmStarter() {
	// TODO Auto-generated destructor stub
}

void FarmStarter::infoHandler() {
	DimInfo *curr = getInfo();
	// get current DimInfo address
	if (curr == &availableSourceIDs_) {
		char* str = availableSourceIDs_.getString();
		std::cerr << "L0DataSourceIDs=" << str << std::endl;
		//		std::string enabledDetectorIDs(str);
		//		killFarm();
		//		startFarm("L0DataSourceIDs=0x04:1,0x08:1,0x0C:1,0x10:1,0x14:1,0x18:1,0x1C:1,0x20:1,0x28:1,");
	}
}

void FarmStarter::startFarm(std::string param) {
	std::cout << "Starting farm program!" << std::endl;
	farmPID_ = fork();
	if (farmPID_ == 0) {
		boost::filesystem::path execPath(Options::FARM_EXEC_PATH);

		char * argv[2];
		argv[0] = (char*) execPath.filename().string().data();
		argv[1] = (char*) param.data();
		execvp(execPath.string().data(), argv);
	} else if (farmPID_ == -1) {
		std::cerr << "Forking failed! Unable to start the farm program!"
				<< std::endl;
	}
}

void FarmStarter::killFarm() {
	if (farmPID_ > 0) {
		kill(farmPID_, SIGKILL);
	}
}
} /* namespace dim */
} /* namespace na62 */
