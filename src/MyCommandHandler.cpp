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
	//if (Options::GetInt(OPTION_VERBOSITY) != 0) {
		LOG_INFO( "Command received message: " + message);
	//}

	std::transform(message.begin(), message.end(), message.begin(), ::tolower);
	if (message == "start") {
		LOG_INFO("Start received");
		farmStarter_.startFarm();
	} else if (message == "restart") {
		LOG_INFO("Restart received");
		farmStarter_.restartFarm();
	} else if (message == "stop") {
		LOG_INFO("Stop received");
		farmStarter_.killFarm();
	} else if (message == "singleprocess") {
			LOG_INFO("Singleprocess Farm received");
			farmStarter_.setSingleProcess();
	} else if (message == "multiprocess") {
			LOG_INFO("Multiprocess Farm received (Shared Memory)");
			farmStarter_.setMultiProcess();
	} else if (message == "test") {
		LOG_INFO("Test received");
		farmStarter_.test();
	} else {
		LOG_INFO( "Sending command: " + message);
		messageQueueConnector_->sendCommand(message);
	}
}
} /* namespace dim */
} /* namespace na62 */
