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
	lastSentState_ = OFF;

	while (true) {
		STATE state = OFF;

		std::string statisticsMessage;

		IPCHandler::setTimeout(
				Options::GetInt(OPTION_HEARTBEAT_TIMEOUT_MILLIS));
		while (true) {

			state = IPCHandler::tryToReceiveState();
			while (state != RUNNING && state != TIMEOUT) {
				LOG_INFO("Received heart beat: setting state to " + state);
				if (lastSentState_ != state && state != TIMEOUT) {
					sendState(state);
					lastSentState_ = state;
				}
				state = IPCHandler::tryToReceiveState();
			}

			if (state != TIMEOUT) {
				LOG_INFO("Received heart beat: setting state to " + state);
				if (lastSentState_ != state) {
					sendState(state);
					lastSentState_ = state;
				}

				while (!(statisticsMessage =
						IPCHandler::tryToReceiveStatistics()).empty()) {
					if (Options::GetInt(OPTION_VERBOSITY) > 1) {
						LOG_INFO("Received: " + statisticsMessage);
					}

					std::string statisticsName = statisticsMessage.substr(0,
							statisticsMessage.find(':'));
					std::string statistics = statisticsMessage.substr(
							statisticsMessage.find(':') + 1);
					try {
						if (statisticsName.find("EOBStatsL1") != std::string::npos) {
							dimServer_->updateL1EOBStatistics(statisticsName, statistics);
						} else if (statisticsName.find("EOBStatsL2") != std::string::npos) {
							dimServer_->updateL2EOBStatistics(statisticsName, statistics);
						} else {
							if (statistics.find(";") != std::string::npos) { // separated key/value pairs
								dimServer_->updateStatistics(statisticsName,
										statistics);
							} else if (statistics.find("{")
									!= std::string::npos) { // JSON string
								dimServer_->updateStatistics(statisticsName,
										statistics);
							} else {
								dimServer_->updateStatistics(statisticsName,
										boost::lexical_cast<longlong>(
												statistics));
							}
						}
					} catch (boost::bad_lexical_cast const& e) {
						LOG_ERROR(
								"Bad format of message for service "
										+ statisticsName + ": "
										+ statisticsMessage);
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
				LOG_INFO("Heart beat timeout: setting state to OFF");
				if (lastSentState_ != OFF) {
					sendState(OFF);
					lastSentState_ = OFF;
				}
			}
		}
		sendState(OFF);
	}
}

void MessageQueueConnector::sendState(STATE state) {
	dimServer_->updateState(state);
}

void MessageQueueConnector::sendCommand(std::string command) {
	//if (lastSentState_ != RUNNING) {
	if (lastSentState_ == INITIALIZING) {
		//I just send the UpdateNextBurstID Command
		if (command.find("UpdateNextBurstID") != std::string::npos) {
			//Do nothing
		} else {
			LOG_ERROR("Command: " + command +  " Not send because the farm state was: " << lastSentState_);
			return;
		}
	}
	IPCHandler::sendCommand(command);

}
} /* namespace dim */
} /* namespace na62 */
