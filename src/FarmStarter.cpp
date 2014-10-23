/*
 * FarmStarter.cpp
 *
 *  Created on: Sep 12, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "FarmStarter.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <monitoring/IPCHandler.h>
#include <functional>

#include "exceptions/NA62Error.h"
#include "options/MyOptions.h"

namespace na62 {
namespace dim {

FarmStarter::FarmStarter(MessageQueueConnector_ptr myConnector) :
		availableSourceIDs_("RunControl/EnabledDetectors", -1, this), activeCREAMS_(
				"RunControl/CREAMCrates", -1, this), additionalOptions_("RunControl/PCFarmOptions", -1, this),
				farmPID_(-1), myConnector_(myConnector) {

	dimListener.registerNextBurstNumberListener([this](uint nextBurst) {
		myConnector_->sendCommand(
				"UpdateNextBurstID:"
				+ boost::lexical_cast<std::string>(nextBurst));});

	dimListener.registerRunNumberListener([this](uint runNumber) {
		myConnector_->sendCommand(
				"UpdateRunNumber:"
				+ boost::lexical_cast<std::string>(runNumber));});

	dimListener.registerBurstNumberListener([this](uint burstID) {
		myConnector_->sendCommand(
				"UpdateBurstID:"
				+ boost::lexical_cast<std::string>(burstID));});

	dimListener.registerSobListener([this](uint sob) {
		myConnector_->sendCommand(
				"SOB_Timestamp:"
				+ boost::lexical_cast<std::string>(sob));});

	dimListener.registerEobListener([this](uint eob) {
		myConnector_->sendCommand(
				"EOB_Timestamp:"
				+ boost::lexical_cast<std::string>(eob));});
}

FarmStarter::~FarmStarter() {
	// TODO Auto-generated destructor stub
}

std::vector<std::string> FarmStarter::generateStartParameters() {
	std::vector<std::string> argv;
	if (Options::GetBool(OPTION_IS_MERGER)) {
		/*
		 * Merger
		 */
		int runNumber = dimListener.getRunNumber(); // This should always be 0 unless the PC starts during a run!

		argv.push_back(
				"--currentRunNumber="
						+ boost::lexical_cast<std::string>(runNumber));
		return argv;
	} else {
		/*
		 * PC Farm
		 */
		int currentBurstNum = dimListener.getBurstNumber(); // This should always be 0 unless the PC starts during a run!

		argv.push_back(
				"--firstBurstID="
						+ boost::lexical_cast<std::string>(currentBurstNum));

		argv.push_back("--incrementBurstAtEOB=false"); // Use the nextBurstNumber service to change the burstID instead of just incrementing at EOB

		std::string enabledDetectorIDs = "";
		if (availableSourceIDs_.getSize() <= 0) {
			std::cerr
					<< "Unable to connect to EnabledDetectors service. Unable to start!"
					<< std::endl;
		} else {
			if (availableSourceIDs_.getString()[0] == (char) 0xFFFFFFFF
					&& availableSourceIDs_.getSize() == 4) {
				std::cerr
						<< "EnabledDetectors is empty. Starting the pc-farm will fail!"
						<< std::endl;
			} else {
				char* str = availableSourceIDs_.getString();
				enabledDetectorIDs = std::string(str,
						availableSourceIDs_.getSize());
			}
		}

		std::string creamCrates = "";
		if (activeCREAMS_.getSize() <= 0) {
			std::cerr
					<< "Unable to connect to EnabledDetectors service. Unable to start!"
					<< std::endl;
		} else {
			if (activeCREAMS_.getString()[0] == (char) 0xFFFFFFFF
					&& activeCREAMS_.getSize() == 4) {
				creamCrates = "0:0";
			} else {
				char* str = activeCREAMS_.getString();
				creamCrates = std::string(str, activeCREAMS_.getSize());
			}
		}

		argv.push_back("--L0DataSourceIDs=" + enabledDetectorIDs);
		argv.push_back("--CREAMCrates=" + creamCrates);

		std::string additionalOptions = "";
		if (additionalOptions_.getSize() <= 0) {
			std::cerr
					<< "Unable to connect to AdditionalOptions service. Unable to start!"
					<< std::endl;
		} else {
			if (additionalOptions_.getString()[0] == (char) 0xFFFFFFFF
					&& additionalOptions_.getSize() == 4) {
				std::cerr
						<< "Additional options is empty."
						<< std::endl;
			} else {
				char* str = additionalOptions_.getString();
				additionalOptions = std::string(str,
						additionalOptions_.getSize());
				argv.push_back(additionalOptions);
			}
		}

		return argv;
	}
}

void FarmStarter::infoHandler() {
}

void FarmStarter::startFarm() {
	try {
		startFarm(generateStartParameters());
		//myConnector_->sendState(OFF);
	} catch (NA62Error const& e) {
		std::cerr << e.what() << std::endl;
	}
}

void FarmStarter::restartFarm() {
	killFarm();
	try {
		startFarm(generateStartParameters());
		//myConnector_->sendState(OFF);
	} catch (NA62Error const& e) {
		std::cerr << e.what() << std::endl;
	}
}

void FarmStarter::startFarm(std::vector<std::string> params) {
	std::cout << "Starting farm with following parameters: ";
	for (std::string param : params) {
		std::cout << param << " ";
	}
	std::cout << std::endl;

	if (Options::GetBool(OPTION_IS_MERGER)) {
		sleep(1);
	}

//	if (farmPID_ > 0) {
//		myConnector_->sendState(OFF);
//		return;
//	}

	if (farmPID_ > 0) {
		killFarm();
		sleep(3);
	}

	farmPID_ = fork();
	if (farmPID_ == 0) {
		boost::filesystem::path execPath(
				Options::GetString(OPTION_FARM_EXEC_PATH));

		std::cout << "Starting farm program " << execPath.string() << std::endl;

		char* argv[params.size() + 2];
		argv[0] = (char*) execPath.filename().string().data();

		for (unsigned int i = 0; i < params.size(); i++) {
			argv[i + 1] = (char*) params[i].data();
		}
		argv[params.size() + 1] = NULL;

		execv(execPath.string().data(), argv);
		std::cerr << "Main farm program stopped!" << std::endl;
		farmPID_ = -1;

		exit(0);
	} else if (farmPID_ == -1) {
		std::cerr << "Forking failed! Unable to start the farm program!"
				<< std::endl;
	}
	//myConnector_->sendState(OFF);
}

void FarmStarter::killFarm() {
	boost::filesystem::path execPath(Options::GetString(OPTION_FARM_EXEC_PATH));
	std::cerr << "Killing " << execPath.filename() << std::endl;

	signal(SIGCHLD, SIG_IGN);
	if (farmPID_ > 0) {
		kill(farmPID_, SIGTERM);
//		wait((int*) NULL);
//		waitpid(farmPID_, 0,WNOHANG);
	}
	system(std::string("killall -9 " + execPath.filename().string()).data());
	farmPID_ = 0;
}
} /* namespace dim */
} /* namespace na62 */
