/*
 * MyCommandHandler.h
 *
 *  Created on: Jun 21, 2012
 *      Author: kunzejo
 */

#ifndef MYCOMMANDHANDLER_H_
#define MYCOMMANDHANDLER_H_
#include <algorithm>
#include <dim/dis.hxx>
#include <iostream>

#include "messages/MessageHandler.h"
#include "FarmStarter.h"
#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {
class MyCommandHandler: public DimCommandHandler // In order to inherit "commandHandler"
{
public:
	// The constructor creates the Commands
	MyCommandHandler(std::string& hostname,
			MessageQueueConnector_ptr messageQueueConnector,
			FarmStarter& farmStarter) :
			runNumber(
					new DimCommand((hostname + std::string("/CMD")).data(),
							(char*) "C", this)), messageQueueConnector_(
					messageQueueConnector), farmStarter_(farmStarter) {
	}

	// Overloaded method commandHandler called whenever commands arrive,
	void commandHandler();

private:
	DimCommand *runNumber;
	MessageQueueConnector_ptr messageQueueConnector_;
	FarmStarter& farmStarter_;

};
} /* namespace dim */
} /* namespace na62 */
#endif /* MYCOMMANDHANDLER_H_ */
