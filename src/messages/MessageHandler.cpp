/*
 * MessageHandler.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: kunzejo
 */

#include "../options/Options.h"
#include "MessageHandler.h"

namespace na62 {

boost::mutex MessageHandler::echoMutex;
//SQLConnector* MessageHandler::sqlConnector = NULL;

void MessageHandler::Write(const std::string& message) {
	Write(message, "Message");
}

void MessageHandler::Write(const std::string& message,
		const std::string& tableName) {
	boost::lock_guard<boost::mutex> lock(echoMutex); // Will lock until return
	std::cerr << message << std::endl;
}

} /* namespace na62 */
