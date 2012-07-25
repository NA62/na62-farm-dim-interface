/*
 * MyCommandHandler.h
 *
 *  Created on: Jun 21, 2012
 *      Author: kunzejo
 */

#ifndef MYCOMMANDHANDLER_H_
#define MYCOMMANDHANDLER_H_

#include <dim/dis.hxx>
#include <iostream>

#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {
class MyCommandHandler: public DimCommandHandler // In order to inherit "commandHandler"
{
private:
	DimCommand *runNumber;
	MessageQueueConnector_ptr messageQueueConnector_;

public:
	// Overloaded method commandHandler called whenever commands arrive,
	void commandHandler() {
		DimCommand *currCmnd = getCommand();

		std::string message;
		message.resize(currCmnd->getSize());
		message = std::string(currCmnd->getString());
		std::cout << "Received message: " << message << std::endl;
		messageQueueConnector_->sendCommand(message);
	}

	// The constructor creates the Commands
	MyCommandHandler(std::string& hostname,
			MessageQueueConnector_ptr messageQueueConnector) :
			runNumber(
					new DimCommand((hostname + std::string("/CMD")).data(),
							(char*) "C", this)), messageQueueConnector_(
					messageQueueConnector) {
	}
};
} /* namespace dim */
} /* namespace na62 */
#endif /* MYCOMMANDHANDLER_H_ */
