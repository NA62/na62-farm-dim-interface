/*
 * FarmStarter.h
 *
 *  Created on: Sep 12, 2012
 *      Author: root
 */

#ifndef FARMSTARTER_H_
#define FARMSTARTER_H_

#include <dim/dic.hxx>

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
	std::string genrateStartParameters();
	void startFarm(std::string param);

	void infoHandler();

	DimInfo availableSourceIDs_;
	DimInfo burstNumber_;
	DimInfo runNumber_;
	pid_t farmPID_;
	MessageQueueConnector_ptr myConnector_;
};
} /* namespace dim */
} /* namespace na62 */
#endif /* FARMSTARTER_H_ */
