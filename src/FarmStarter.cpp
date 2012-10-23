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

FarmStarter::FarmStarter(MessageQueueConnector_ptr myConnector) :
		availableSourceIDs_("RunControl/EnabledDetectors", -1, this), burstNumber_(
				"RunControl/BurstNumber", -1, this), runNumber_(
				"RunControl/RunNumber", -1, this), farmPID_(-1), myConnector_(
				myConnector) {
}

FarmStarter::~FarmStarter() {
	// TODO Auto-generated destructor stub
}

std::string FarmStarter::genrateStartParameters() {
	if (Options::IS_MERGER) {
		/*
		 * Merger
		 */
		return "";
	} else {
		/*
		 * PC Farm
		 */
		int currentBurstNum = 0;
		if (burstNumber_.getSize() <= 0) {
			throw NA62Error(
					"Unable to connect to RunNumber service! Refusing to start.");
		} else {
			currentBurstNum = burstNumber_.getInt(); // This should always be 0 unless the PC starts during a run!
		}

		int runNumber = 0;
		if (runNumber_.getSize() <= 0) {
			throw NA62Error(
					"Unable to connect to RunNumber service! Refusing to start.");
		} else {
			runNumber = runNumber_.getInt(); // This should always be 0 unless the PC starts during a run!
		}

		std::string enabledDetectorIDs = "";
		if (availableSourceIDs_.getSize() <= 0) {
			throw NA62Error(
					"Unable to connect to EnabledDetectors  service. Unable to start!");
		} else {
			char* str = availableSourceIDs_.getString();
			enabledDetectorIDs = std::string(str);
			if (enabledDetectorIDs == "") {
				throw NA62Error("No Detectors are enabled! Refusing to start.");
			}
		}

		return std::string("--L0DataSourceIDs=") + enabledDetectorIDs + " "
				+ "--firstBurstID="
				+ boost::lexical_cast<std::string>(currentBurstNum) + " "
				+ +"--currentRunNumber="
				+ boost::lexical_cast<std::string>(runNumber);
	}
}

void FarmStarter::infoHandler() {
	DimInfo *curr = getInfo();
	// get current DimInfo address
	if (curr == &runNumber_) {
		int runNumber = runNumber_.getInt();
		mycout << "Updating RunNumber to " << runNumber << std::endl;
		myConnector_->sendCommand(
				"UpdateRunNumber:"
						+ boost::lexical_cast<std::string>(runNumber));
	}
}

void FarmStarter::startFarm() {
	try {
		startFarm(genrateStartParameters());
	} catch (NA62Error const& e) {
		mycerr << e.what() << std::endl;
	}
}

void FarmStarter::restartFarm() {
	killFarm();
	try {
		startFarm(genrateStartParameters());
	} catch (NA62Error const& e) {
		mycerr << e.what() << std::endl;
	}
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
