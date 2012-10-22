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

std::string FarmStarter::genrateStartParameters() {
	char* str = availableSourceIDs_.getString();
	std::string enabledDetectorIDs(str);
	return std::string("L0DataSourceIDs=") + enabledDetectorIDs;
}

void FarmStarter::infoHandler() {
	DimInfo *curr = getInfo();
	// get current DimInfo address
	if (curr == &availableSourceIDs_) {
		char* str = availableSourceIDs_.getString();
		std::string enabledDetectorIDs(str);
//		killFarm();
//		startFarm(std::string("L0DataSourceIDs=") + enabledDetectorIDs);
	}
}

void FarmStarter::startFarm() {
	startFarm(genrateStartParameters());
}

void FarmStarter::restartFarm() {
	killFarm();
	startFarm(genrateStartParameters());
}

void FarmStarter::startFarm(std::string param) {
	farmPID_ = fork();
	mycout << "Forked: " << farmPID_ << std::endl;
	if (farmPID_ == 0) {
		boost::filesystem::path execPath(Options::FARM_EXEC_PATH);

		mycout << "Starting farm program " << execPath.string() << std::endl;
		mycerr << (const char*) execPath.filename().string().data()
				<< (const char*) param.data() << std::endl;

		execl(execPath.string().data(), execPath.filename().string().data(),
				param.data(), NULL);
		mycerr << "Main farm program stopped!" << std::endl;
		farmPID_ = -1;
		exit(1);
	} else if (farmPID_ == -1) {
		mycerr << "Forking failed! Unable to start the farm program!"
				<< std::endl;
	}
}

void FarmStarter::killFarm() {
	if (farmPID_ > 0) {
		kill(farmPID_, SIGKILL);
	}
	sleep(1);
	boost::filesystem::path execPath(Options::FARM_EXEC_PATH);
	std::cerr << "Killing " << execPath.filename() << std::endl;
	system(std::string("killall -9 " + execPath.filename().string()).data());
}
} /* namespace dim */
} /* namespace na62 */
