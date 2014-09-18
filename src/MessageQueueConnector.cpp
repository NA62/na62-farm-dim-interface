/*
 * MessageQueueConnector.cpp
 *
 *  Created on: Jul 11, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */
#include <boost/lexical_cast.hpp>
#include <monitoring/IPCHandler.h>

#include "MonitorDimServer.h"
#include "options/MyOptions.h"

#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {

MessageQueueConnector::MessageQueueConnector() {
	// TODO Auto-generated constructor stub

}

MessageQueueConnector::~MessageQueueConnector() {
	// TODO Auto-generated destructor stub
}

void MessageQueueConnector::run() {
	STATE lastSentState = OFF;
	while (true) {
		STATE state = OFF;

		std::string statisticsMessage;

		IPCHandler::setTimeout(
				Options::GetInt(OPTION_HEARTBEAT_TIMEOUT_MILLIS));
		while (true) {

			state = IPCHandler::tryToReceiveState();

			if (state != TIMEOUT) {
				std::cerr << "Received heart beat: setting state to " << state
						<< std::endl;
				if (lastSentState != state) {
					sendState(state);
					lastSentState = state;
				}

				while (!(statisticsMessage =
						IPCHandler::tryToReceiveStatistics()).empty()) {
					if (Options::GetInt(OPTION_VERBOSITY) != 0) {
						std::cout << "Received: " << statisticsMessage
								<< std::endl;
					}

					std::string statisticsName = statisticsMessage.substr(0,
							statisticsMessage.find(':'));
					std::string statistics = statisticsMessage.substr(
							statisticsMessage.find(':') + 1);

					try {
						if (statisticsName == "ErrorMessage") {
							dimServer_->updateErrorMessage(statistics);
						} else if (statistics.find(";") != std::string::npos) { // separated key/value pairs
							dimServer_->updateStatistics(statisticsName,
									statistics);
						} else {
							dimServer_->updateStatistics(statisticsName,
									boost::lexical_cast<longlong>(statistics));
						}
					} catch (boost::bad_lexical_cast const& e) {
						std::cout << "Bad format of message for service "
								<< statisticsName << ": " << statisticsMessage
								<< std::endl;
					}
				}
			} else {
				std::cerr << "Heart beat timeout: setting state to OFF"
						<< std::endl;
				if (lastSentState != OFF) {
					sendState(OFF);
					lastSentState = OFF;
				}
			}
		}
		sendState(OFF);
		std::cout << "done" << std::endl;
	}
}

void MessageQueueConnector::sendState(STATE state) {
	dimServer_->updateState(state);
}

void MessageQueueConnector::sendCommand(std::string command) {
	IPCHandler::sendCommand(command);
}
} /* namespace dim */
} /* namespace na62 */
