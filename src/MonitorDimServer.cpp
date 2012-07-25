/*
 * MonitorDimServer.cpp
 *
 *  Created on: Jun 20, 2012
 *      Author: kunzejo
 */

#include "MessageQueueConnector.h"
#include "MonitorDimServer.h"

namespace na62 {
namespace dim {

MonitorDimServer::MonitorDimServer(
		MessageQueueConnector_ptr messageQueueConnector, std::string hostName) :
		cmdh(hostName, messageQueueConnector), initialState(OFF), stateService(
				std::string(hostName + "/State").data(), initialState), statisticsService(
				std::string(hostName + "/Statistics").data(), ""), messageQueueConnector_(
				messageQueueConnector) {
	start(hostName.data());
}

MonitorDimServer::~MonitorDimServer() {
	stop();
}

void MonitorDimServer::updateState(STATE state) {
	int state_int = (int) state;
	stateService.updateService(state_int);
}

void MonitorDimServer::updateStatistics(std::string statistics) {
	statisticsService.updateService((char*) statistics.data());
}

} /* namespace dim */
} /* namespace na62 */
