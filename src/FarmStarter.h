/*
 * FarmStarter.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef FARMSTARTER_H_
#define FARMSTARTER_H_

#include <dim/dic.hxx>
#include <vector>
#include <string>

#include "MessageQueueConnector.h"
namespace na62 {
namespace dim {

class FarmStarter: public DimClient {
public:
	FarmStarter(MessageQueueConnector_ptr myConnector);
	virtual ~FarmStarter();
	void restartFarm();
	void startFarm();
	void killFarm();

private:
	std::vector<std::string> generateStartParameters();
	void startFarm(std::vector<std::string> param);

	void infoHandler();

	DimInfo availableSourceIDs_;
	DimInfo activeCREAMS_;
	DimInfo burstNumber_;
	DimInfo runNumber_;
	DimInfo SOB_TS_;
	pid_t farmPID_;
	MessageQueueConnector_ptr myConnector_;
};
} /* namespace dim */
} /* namespace na62 */
#endif /* FARMSTARTER_H_ */
