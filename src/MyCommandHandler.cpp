/*
 * MyCommandHandler.cpp
 *
 *  Created on: Jun 21, 2012
 *      Author: kunzejo
 */

#include "options/MyOptions.h"
#include "MyCommandHandler.h"

namespace na62 {
namespace dim {

void MyCommandHandler::commandHandler() {
	DimCommand *currCmnd = getCommand();

	std::string message;
	message.resize(currCmnd->getSize());
	message = std::string(currCmnd->getString());
	if (Options::GetInt(OPTION_VERBOSITY) != 0) {
		std::cout << "Received message: " << message << std::endl;
	}

	std::transform(message.begin(), message.end(), message.begin(), ::tolower);
	if (message == "start") {
		std::cout << "Start received" << std::endl;
		farmStarter_.startFarm();
	} else if (message == "restart") {
		std::cout << "Restart received" << std::endl;
		farmStarter_.restartFarm();
	} else if (message == "stop") {
		std::cout << "Stop received" << std::endl;
		farmStarter_.killFarm();
	} else {
		std::cout << "Sending command:" << message << std::endl;
		messageQueueConnector_->sendCommand(message);
	}
}
} /* namespace dim */
} /* namespace na62 */
