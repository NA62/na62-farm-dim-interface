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
#include "utils/AExecutable.h"
#include <boost/filesystem.hpp>

#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {

class FarmStarter: public DimClient, public AExecutable {
public:
	FarmStarter(MessageQueueConnector_ptr myConnector);
	virtual ~FarmStarter();
	void restartFarm();
	void startFarm();
	void killFarm();
	void test();

private:
	std::vector<std::string> generateStartParameters();
	void startFarm(std::vector<std::string> param);
	void startProcessor(std::vector<std::string> params);

	virtual void thread() override;

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
	std::vector<pid_t> processorsPID_;
	MessageQueueConnector_ptr myConnector_;

	DimListener dimListener;
};
} /* namespace dim */
} /* namespace na62 */
#endif /* FARMSTARTER_H_ */
