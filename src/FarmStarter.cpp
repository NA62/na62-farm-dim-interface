/*
 * FarmStarter.cpp
 *
 *  Created on: Sep 12, 2012
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <wait.h>

#include "options/Options.h"

#include "FarmStarter.h"

namespace na62 {
namespace dim {

FarmStarter::FarmStarter(MessageQueueConnector_ptr myConnector) :
		availableSourceIDs_("RunCOntrol/EnabledDetectors", -1, this), burstNumber_(
				"RunControl/BurstNumber", -1, this), runNumber_(
				"RunControl/RunNumber", -1, this), SOB_TS_("NA62/Timing/SOB", 0,
				this), farmPID_(-1), myConnector_(myConnector) {
}

FarmStarter::~FarmStarter() {
	// TODO Auto-generated destructor stub
}

std::vector<std::string> FarmStarter::generateStartParameters() {
	std::vector < std::string > argv;
	if (Options::IS_MERGER) {
		/*
		 * Merger
		 */
		int runNumber = 0;
		if (runNumber_.getSize() <= 0) {
			throw NA62Error(
					"Unable to connect to RunNumber service! Refusing to start.");
		} else {
			runNumber = runNumber_.getInt(); // This should always be 0 unless the PC starts during a run!
		}

		argv.push_back(
				"--currentRunNumber="
						+ boost::lexical_cast<std::string>(runNumber));
		return argv;
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

		std::string enabledDetectorIDs = "";
		if (availableSourceIDs_.getSize() <= 0) {
			throw NA62Error(
					"Unable to connect to EnabledDetectors  service. Unable to start!");
		} else {
			char* str = availableSourceIDs_.getString();
			enabledDetectorIDs = std::string(str,
					availableSourceIDs_.getSize());

			std::cout << availableSourceIDs_.getSize() << std::endl;
			std::cout << str<< std::endl;
			std::cout << enabledDetectorIDs<< std::endl;
			if (enabledDetectorIDs == "") {
				throw NA62Error("No Detectors are enabled! Refusing to start.");
			}
		}

		argv.push_back("--L0DataSourceIDs=" + enabledDetectorIDs);
		argv.push_back(
				"--firstBurstID="
						+ boost::lexical_cast<std::string>(currentBurstNum));
		return argv;
	}
}

void FarmStarter::infoHandler() {
	DimInfo *curr = getInfo();
	if (curr == &runNumber_) {
		int runNumber = runNumber_.getInt();
		std::cout << "Updating RunNumber to " << runNumber << std::endl;
		myConnector_->sendCommand(
				"UpdateRunNumber:"
						+ boost::lexical_cast<std::string>(runNumber));
	} else if (curr == &SOB_TS_) {
		uint32_t timestamp = SOB_TS_.getInt();
		std::cout << "Updating SOB timestamp to " << timestamp << std::endl;
		myConnector_->sendCommand(
				"SOB_Timestamp:" + boost::lexical_cast<std::string>(timestamp));
	} else if (curr == &burstNumber_) {

		uint32_t burst = burstNumber_.getInt();
		std::cout << "Updating burst ID to " << burst << std::endl;
		myConnector_->sendCommand(
				"UpdateBurstID:" + boost::lexical_cast<std::string>(burst));
	}
}

void FarmStarter::startFarm() {
	try {
		startFarm(generateStartParameters());
	} catch (NA62Error const& e) {
		mycerr << e.what() << std::endl;
	}
}

void FarmStarter::restartFarm() {
	killFarm();
	try {
		startFarm(generateStartParameters());
	} catch (NA62Error const& e) {
		mycerr << e.what() << std::endl;
	}
}

void FarmStarter::startFarm(std::vector<std::string> params) {
	if (Options::IS_MERGER) {
		sleep(1);
	}

	if (farmPID_ > 0) {
		return;
	}

	killFarm();

	farmPID_ = fork();
	mycout << "Forked: " << farmPID_ << std::endl;
	if (farmPID_ == 0) {
		boost::filesystem::path execPath(Options::FARM_EXEC_PATH);

		mycout << "Starting farm program " << execPath.string() << std::endl;

		char* argv[params.size() + 2];
		argv[0] = (char*) execPath.filename().string().data();

		for (unsigned int i = 0; i < params.size(); i++) {
			argv[i + 1] = (char*) params[i].data();
		}
		argv[params.size() + 1] = NULL;

		execv(execPath.string().data(), argv);
		mycerr << "Main farm program stopped!" << std::endl;
		farmPID_ = -1;

		exit(0);
	} else if (farmPID_ == -1) {
		mycerr << "Forking failed! Unable to start the farm program!"
				<< std::endl;
	}
}

void FarmStarter::killFarm() {
	boost::filesystem::path execPath(Options::FARM_EXEC_PATH);
	std::cerr << "Killing " << execPath.filename() << std::endl;

	if (farmPID_ > 0) {
		kill(farmPID_, SIGTERM);
		wait((int*) NULL);
	}
	system(std::string("killall -9 " + execPath.filename().string()).data());
	farmPID_ = 0;
}
} /* namespace dim */
} /* namespace na62 */
