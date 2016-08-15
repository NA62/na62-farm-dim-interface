/*
 * FarmStarter.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef FARMSTARTER_H_
#define FARMSTARTER_H_

#include <dim/dic.hxx>
#include <sys/types.h>
#include <string>
#include <vector>
#include <string>
#include <dim/DimListener.h>
#include <boost/filesystem.hpp>
#include <mutex>

#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {



class FarmStarter : public DimClient {
public:
	FarmStarter(MessageQueueConnector_ptr myConnector);
	virtual ~FarmStarter();
	void restartFarm();
	void startFarm();
	void killFarm();
	void test();

	void startProcessors(int amount);

	std::vector<pid_t> inline  getProcessorPID(){
		return processorsPID_;
	}

	int inline getProcessorAmount() {
		return processorAmount_;
	}
	void inline pushPID(pid_t child_pid) {
		mtx.lock(); //Static variable can be modifiable from DIM Server and the monitor Thread
	processorsPID_.push_back(child_pid);
		mtx.unlock();
	}
	bool inline getMonitoringStatus() {
		return monitoringStatus_;
	}
	std::vector<std::string> generateStartParameters();
private:

	void startFarm(std::string path, std::vector<std::string> param);
	void startSharedMemoryFarm(std::vector<std::string> params);
	void startProcessor(std::vector<std::string> params);
	void startCleaner(std::string path, std::vector<std::string> params);
	void killFarm(std::string path);
	void killSharedMemoryFarm();
	void killProcessors();


	int launchExecutable(boost::filesystem::path  execPath, std::vector<std::string> params);
	char ** generateArgv(boost::filesystem::path execPath, std::vector<std::string> params);


	void infoHandler();
	std::string myIP_;
	DimInfo availableSourceIDs_;
	DimInfo availableL1SourceIDs_;
	DimInfo mepFactor_;
	DimInfo enabledPCNodes_;
	DimInfo enabledMergerNodes_;
	DimInfo additionalOptions_;
	pid_t farmPID_;
	bool monitoringStatus_ = 0; //1 enabled 0 disabled

	MessageQueueConnector_ptr myConnector_;

	std::mutex mtx;
	int processorAmount_ = 5;
	std::vector<pid_t> processorsPID_;

	DimListener dimListener;
};
} /* namespace dim */
} /* namespace na62 */
#endif /* FARMSTARTER_H_ */
