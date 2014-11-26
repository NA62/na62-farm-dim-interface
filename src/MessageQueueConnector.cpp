/*
 * MessageQueueConnector.cpp
 *
 *  Created on: Jul 11, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */
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
			while (state != RUNNING && state != TIMEOUT) {
				LOG_ERROR << "Received heart beat: setting state to " << state
						<< ENDL;
				if (lastSentState != state && state != TIMEOUT) {
					sendState(state);
					lastSentState = state;
				}
				state = IPCHandler::tryToReceiveState();
			}

			if (state != TIMEOUT) {
				LOG_ERROR << "Received heart beat: setting state to " << state
						<< ENDL;
				if (lastSentState != state) {
					sendState(state);
					lastSentState = state;
				}

				while (!(statisticsMessage =
						IPCHandler::tryToReceiveStatistics()).empty()) {
					if (Options::GetInt(OPTION_VERBOSITY) > 1) {
						LOG_INFO << "Received: " << statisticsMessage << ENDL;
					}

					std::string statisticsName = statisticsMessage.substr(0,
							statisticsMessage.find(':'));
					std::string statistics = statisticsMessage.substr(
							statisticsMessage.find(':') + 1);

					try {
						if (statistics.find(";") != std::string::npos) { // separated key/value pairs
							dimServer_->updateStatistics(statisticsName,
									statistics);
						} else {
							dimServer_->updateStatistics(statisticsName,
									boost::lexical_cast<longlong>(statistics));
						}
					} catch (boost::bad_lexical_cast const& e) {
						LOG_ERROR << "Bad format of message for service "
								<< statisticsName << ": " << statisticsMessage
								<< ENDL;
					}

					int counter = 0;
					IPCHandler::setTimeout(10);
					while (state != TIMEOUT && counter < 1e5) {
						state = IPCHandler::tryToReceiveState();
						counter++;
					}
					IPCHandler::setTimeout(
							Options::GetInt(OPTION_HEARTBEAT_TIMEOUT_MILLIS));
				}
				IPCHandler::setTimeout(10);
				while (state != TIMEOUT) {
					state = IPCHandler::tryToReceiveState();
				}
				IPCHandler::setTimeout(
						Options::GetInt(OPTION_HEARTBEAT_TIMEOUT_MILLIS));
			} else {
				LOG_INFO << "Heart beat timeout: setting state to OFF" << ENDL;
				if (lastSentState != OFF) {
					sendState(OFF);
					lastSentState = OFF;
				}
			}
		}
		sendState(OFF);
		LOG_INFO << "done" << ENDL;
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
