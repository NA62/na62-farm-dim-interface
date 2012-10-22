/*
 * MyCommandHandler.cpp
 *
 *  Created on: Jun 21, 2012
 *      Author: kunzejo
 */

#include "MyCommandHandler.h"

namespace na62 {
namespace dim {

void MyCommandHandler::commandHandler() {
	DimCommand *currCmnd = getCommand();

	std::string message;
	message.resize(currCmnd->getSize());
	message = std::string(currCmnd->getString());
	mycout << "Received message: " << message << std::endl;

	std::transform(message.begin(), message.end(), message.begin(), ::tolower);
	if (message == "start") {
		farmStarter_.startFarm();
	} else if (message == "restart") {
		farmStarter_.restartFarm();
	} else if (message == "stop") {
		farmStarter_.killFarm();
	} else {
		messageQueueConnector_->sendCommand(message);
	}
}
} /* namespace dim */
} /* namespace na62 */
