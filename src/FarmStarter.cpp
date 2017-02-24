/*
 * FarmStarter.cpp
 *
 *  Created on: Sep 12, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include <boost/algorithm/string.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <monitoring/IPCHandler.h>
#include <functional>

#include "FarmStarter.h"
#include "exceptions/NA62Error.h"
#include "options/MyOptions.h"
#include "SharedMemory/SharedMemoryManager.h"

namespace na62 {
namespace dim {

FarmStarter::FarmStarter(MessageQueueConnector_ptr myConnector) :
		availableSourceIDs_("RunControl/EnabledDetectors", -1, this), availableL1SourceIDs_("RunControl/L1EnabledDetectors", -1, this),
				mepFactor_("RunControl/MEPFactor", -1, this),
				enabledPCNodes_("RunControl/EnabledPCNodes", -1, this), enabledMergerNodes_("RunControl/EnabledMergers", -1, this),
				additionalOptions_("RunControl/PCFarmOptions", -1, this), farmPID_(-1), myConnector_(myConnector) {

	dimListener.registerNextBurstNumberListener([this](uint nextBurst) {
		myConnector_->sendCommand(
				"UpdateNextBurstID:"
				+ std::to_string(nextBurst));});

	dimListener.registerRunNumberListener([this](uint runNumber) {
		myConnector_->sendCommand(
				"UpdateRunNumber:"
				+ std::to_string(runNumber));});

	dimListener.registerBurstNumberListener([this](uint burstID) {
		myConnector_->sendCommand(
				"UpdateBurstID:"
				+ std::to_string(burstID));});

	dimListener.registerSobListener([this](uint sob) {
		myConnector_->sendCommand(
				"SOB_Timestamp:"
				+ std::to_string(sob));});

	dimListener.registerEobListener([this](uint eob) {
		myConnector_->sendCommand(
				"EOB_Timestamp:"
				+ std::to_string(eob));});

	/*
	dimListener.registerRunningMergerListener([this](std::string mergers) {
		if(mergers.empty()) {
			myConnector_->sendCommand(
					"RunningMergers:"+mergers);
		}
	});
	*/
	signal(SIGCHLD, SIG_IGN);

	if (Options::GetBool(OPTION_IS_SHARED_MEMORY)) {
		isSharedMemory_ = 1;
	} else {
		isSharedMemory_ = 0;
	}
}

FarmStarter::~FarmStarter() {
	// TODO Auto-generated destructor stub
}

void FarmStarter::setSingleProcess() {
	isSharedMemory_ = 0;
}

void FarmStarter::setMultiProcess() {
	isSharedMemory_ = 1;
}

void FarmStarter::test() {
	startProcessors(processorAmount_, generateStartParameters("trigger-processor"));
	monitoringStatus_ = 1;
	myConnector_->sendCommand("RunningMergers:" + dimListener.getBurstNumber());
}

void FarmStarter::startFarm() {
	try {
		if (isSharedMemory_) {
			startSharedMemoryFarm();
		} else {
			startFarm(Options::GetString(OPTION_FARM_EXEC_PATH), generateStartParameters("na62-farm"));
		}
	} catch (NA62Error const& e) {
		LOG_ERROR( e.what());
	}
}

void FarmStarter::restartFarm() {
	killFarm();
	try {
		killFarm();
		sleep(3);
		startFarm();
	} catch (NA62Error const& e) {
		LOG_ERROR( e.what());
	}
}
void FarmStarter::startProcessors(int amount, std::vector<std::string> params) {
	LOG_INFO("Starting processors!!");
	for (int i = 0; i < amount; i++) {
		startProcessor(Options::GetString(OPTION_TRIGGER_PROCESSOR_EXEC_PATH), params);
	}
}

void FarmStarter::startProcessor(std::string path, std::vector<std::string> params) {
	boost::filesystem::path exec_path(path);
	LOG_INFO("Starting trigger processor " << exec_path.string());
	signal(SIGCHLD, SIG_IGN);

	pid_t child_pid = fork();
	if (child_pid == 0) {
		std::cout<<"child: "<<child_pid<<" "<<getpid()<<std::endl;
		if (launchExecutable(exec_path, params) < 0) {
			LOG_INFO("Error starting the new process" << exec_path.string());
		}
		LOG_ERROR("Error child not started!!");
		exit(0);
	}
	//Let's say critical
	mtx.lock(); //Static variable can be modifiable from DIM Server and the monitor Thread
	processorsPID_.push_back(child_pid);
	mtx.unlock();
}

void FarmStarter::startFarm(std::string path, std::vector<std::string> params) {

	LOG_INFO("Starting farm process with following parameters: ");
	for (std::string param : params) {
		LOG_INFO(param);
	}

	if (Options::GetBool(OPTION_IS_MERGER)) {
		sleep(1);
	}

	if (farmPID_ > 0) {
		killFarm();
		//sleep(3);
	}

	farmPID_ = fork();
	if (farmPID_ == 0) {
		//boost::filesystem::path execPath(Options::GetString(OPTION_FARM_EXEC_PATH));
		boost::filesystem::path execPath(path);
		LOG_INFO ("Starting farm program: " << execPath.string());
		launchExecutable(execPath, params);

		LOG_INFO("Main farm program stopped!");
		farmPID_ = -1;

		exit(0);
	} else if (farmPID_ == -1) {
		LOG_ERROR("Forking failed! Unable to start the farm program!");
	}
	//myConnector_->sendState(OFF);
}

void FarmStarter::startSharedMemoryFarm() {
	//signal(SIGCHLD, SIG_IGN);

	LOG_INFO("Starting Shared Memory farm: ");
	//Clean Shared memory if any
	na62::SharedMemoryManager::eraseAll();

	sleep(1);
	//Start one processor
	std::vector<std::string> triggerProcessorParams = generateStartParameters("trigger-processor");
	startProcessor(Options::GetString(OPTION_TRIGGER_PROCESSOR_EXEC_PATH), triggerProcessorParams);

	sleep(1);
	//Start farm
	startFarm(Options::GetString(OPTION_SM_FARM_EXEC_PATH), generateStartParameters("na62-farm-sm"));

	//Start the other processors
	startProcessors(processorAmount_ - 1, triggerProcessorParams);
	monitoringStatus_ = 1;
}

void FarmStarter::killFarm() {
	//Don't care about the version
	killSharedMemoryFarm();
	killFarm(Options::GetString(OPTION_FARM_EXEC_PATH));
}

void FarmStarter::killFarm(std::string exec_path) {
	//boost::filesystem::path execPath(Options::GetString(OPTION_FARM_EXEC_PATH));
	boost::filesystem::path execPath(exec_path);
	LOG_INFO("Killing " + exec_path);

	//signal(SIGCHLD, SIG_IGN);
	if (farmPID_ > 0) {
		kill(farmPID_, SIGTERM);
//		wait((int*) NULL);
//		waitpid(farmPID_, 0,WNOHANG);
	}
	sleep(1);
	system(std::string("killall -9 " + execPath.filename().string()).data());
	farmPID_ = 0;
	myConnector_->sendState(OFF);
}

void FarmStarter::killSharedMemoryFarm(){
	killFarm(Options::GetString(OPTION_SM_FARM_EXEC_PATH));
	monitoringStatus_ = 0;
	killProcessors();
	mtx.lock(); //Static variable can be modifiable from DIM Server and the monitor Thread
	processorsPID_.clear();
	mtx.unlock();
}

void FarmStarter::killProcessors() {
	//Killing all stored pid
	for (auto &processor_pid : processorsPID_) {
		if (!kill( (int) processor_pid, 0)) {
			LOG_INFO("Killing: " << processor_pid);
			kill(processor_pid, SIGTERM);
		} else {
			LOG_ERROR(processor_pid << "       dead");
		}
	}
	//Killing any triggerprocess not registed
	std::string path(Options::GetString(OPTION_TRIGGER_PROCESSOR_EXEC_PATH));
	boost::filesystem::path execPath(path);
	LOG_INFO("Killing " + path);
	sleep(1);
	system(std::string("killall -9 " + execPath.filename().string()).data());
}

int FarmStarter::launchExecutable(boost::filesystem::path execPath, std::vector<std::string> params) {

	char* argv[params.size() + 2];
	argv[0] = (char*) execPath.filename().string().data();

	for (unsigned int i = 0; i < params.size(); i++) {
		argv[i + 1] = (char*) params[i].data();
	}
	argv[params.size() + 1] = NULL;

	return execv(execPath.string().data(), argv);
}

std::vector<std::string> FarmStarter::generateStartParameters(std::string appName) {
	std::vector<std::string> argv;
	int runNumber = dimListener.getRunNumber(); // This should always be 0 unless the PC starts during a run!
	argv.push_back("--currentRunNumber=" + std::to_string(runNumber));
	/*
	 * Merger
	 */
	if (Options::GetBool(OPTION_IS_MERGER)) {
		return argv;
	}

	/*
	 * PC Farm
	 */
	argv.push_back("--appName=" + appName); //Useful for log files
	argv.push_back(
			"--firstBurstID="
					+ std::to_string(dimListener.getNextBurstNumber()));

	std::string mergerList;
	if (enabledMergerNodes_.getSize() <= 0) {
		LOG_ERROR("Unable to connect to EnabledMergers service. Unable to start!");
	} else {
		if (enabledMergerNodes_.getString()[0] == (char) 0xFFFFFFFF) {
			LOG_ERROR("EnabledMergerNodes is empty. Starting the pc-farm will fail!");
		} else {
			char* str = enabledMergerNodes_.getString();
			mergerList = std::string(str, enabledMergerNodes_.getSize());
		}
	}
	boost::replace_all(mergerList, ";", ",");
	argv.push_back("--mergerHostNames=" + mergerList);

	std::string farmList = "";
	if (enabledPCNodes_.getSize() <= 0) {
		LOG_ERROR("Unable to connect to EnabledPCNodes service. Unable to start!");
	} else {
		if (enabledPCNodes_.getString()[0] == (char) 0xFFFFFFFF) {
			LOG_ERROR("EnabledPCNodes is empty. Starting the pc-farm will fail!");
		} else {
			char* str = enabledPCNodes_.getString();
			farmList = std::string(str, enabledPCNodes_.getSize());
		}
	}

	boost::replace_all(farmList, ";", ",");
	argv.push_back("--farmHostNames=" + farmList);
	argv.push_back("--numberOfFragmentsPerMEP=" + std::to_string(mepFactor_.getInt()));
	argv.push_back("--incrementBurstAtEOB=0"); // Use the nextBurstNumber service to change the burstID instead of just incrementing at EOB

	std::string enabledDetectorIDs = "";
	if (availableSourceIDs_.getSize() <= 0) {
		LOG_ERROR("Unable to connect to EnabledDetectors service. Unable to start!");
	} else {
		if (availableSourceIDs_.getString()[0] == (char) 0xFFFFFFFF
				&& availableSourceIDs_.getSize() == 4) {
			LOG_ERROR("EnabledDetectors is empty. Starting the pc-farm will fail!");
		} else {
			char* str = availableSourceIDs_.getString();
			enabledDetectorIDs = std::string(str,
					availableSourceIDs_.getSize());
		}
	}

	std::string enabledL1DetectorIDs = "";
	if (availableL1SourceIDs_.getSize() <= 0) {
		LOG_ERROR("Unable to connect to L1EnabledDetectors service. Unable to start!");
	} else {
		if ((availableL1SourceIDs_.getString()[0] == (char) 0xFFFFFFFF || availableL1SourceIDs_.getString()[0] == (char) 0x0 )
				&& availableL1SourceIDs_.getSize() <= 4) {
			LOG_INFO("L1EnabledDetectors is empty.");
		} else {
			char* str = availableL1SourceIDs_.getString();
			enabledL1DetectorIDs = std::string(str, availableL1SourceIDs_.getSize());
			argv.push_back("--L1DataSourceIDs=" + enabledL1DetectorIDs);
		}
	}

	argv.push_back("--L0DataSourceIDs=" + enabledDetectorIDs);

	std::string additionalOptions = "";
	if (additionalOptions_.getSize() <= 0) {
		LOG_ERROR("Unable to connect to AdditionalOptions service. Unable to start!");
	} else {
		if (additionalOptions_.getString()[0] == (char) 0xFFFFFFFF
				&& additionalOptions_.getSize() == 4) {
			LOG_INFO("Additional options is empty.");
		} else {
			char* str = additionalOptions_.getString();
			additionalOptions = std::string(str,
					additionalOptions_.getSize());
			boost::algorithm::trim(additionalOptions);

			std::vector<std::string> strs;
			boost::split(strs, additionalOptions, boost::is_any_of(" "));

			for (auto option : strs) {
				argv.push_back(option);
			}
		}
	}

	for (auto a : argv) {
		LOG_INFO( a );
	}
	return argv;
}

std::string FarmStarter::getSharedProcessorPath() {
	return Options::GetString(OPTION_TRIGGER_PROCESSOR_EXEC_PATH);
}

void FarmStarter::infoHandler() {
}
} /* namespace dim */
} /* namespace na62 */
