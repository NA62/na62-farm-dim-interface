/*
 * MonitorDimServer.cpp
 *
 *  Created on: Jun 20, 2012
 *      Author: kunzejo
 */

#include "options/Options.h"
#include "MessageQueueConnector.h"
#include "MonitorDimServer.h"

namespace na62 {
namespace dim {

MonitorDimServer::MonitorDimServer(
		MessageQueueConnector_ptr messageQueueConnector, std::string hostName,
		FarmStarter& farmStarter) :
		hostName_(hostName), cmdh(hostName, messageQueueConnector, farmStarter), initialState_(
				OFF), stateService_(std::string(hostName + "/State").data(),
				initialState_), errorMessageService_(
				std::string(hostName + "/ErrorMessage").data(), ""), messageQueueConnector_(
				messageQueueConnector) {

	for (unsigned int i = 0; i < Options::MULTI_STAT_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + Options::MULTI_STAT_SERVICES[i]);
		mycout << "Starting service " << serviceName << std::endl;

		DimService_ptr ptr(new DimService(serviceName.data(), (char*) ""));
		multiStatisticServices_[Options::MULTI_STAT_SERVICES[i]] = ptr;
	}

	longlong initialVal = 0;
	for (unsigned int i = 0; i < Options::LONGLONG_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + Options::LONGLONG_SERVICES[i]);
		mycout << "Starting service " << serviceName << std::endl;

		DimService_ptr ptr(new DimService(serviceName.data(), initialVal));
		longlongStatisticServices_[Options::LONGLONG_SERVICES[i]] = ptr;
	}

	start(hostName.data());
}

MonitorDimServer::~MonitorDimServer() {
	stop();
}

void MonitorDimServer::updateState(STATE state) {
	int state_int = (int) state;
	stateService_.updateService(state_int);
}

void MonitorDimServer::updateStatistics(std::string serviceName,
		std::string statistics) {
	if (multiStatisticServices_.find(serviceName)
			== multiStatisticServices_.end()) {
		mycerr << "Unknown service: " << serviceName << std::endl;
		return;
	}
	multiStatisticServices_[serviceName]->updateService(
			(char*) statistics.data());
}

void MonitorDimServer::updateStatistics(std::string serviceName,
		longlong value) {
	if (longlongStatisticServices_.find(serviceName)
			== longlongStatisticServices_.end()) {
		mycerr << "Unknown service: " << serviceName << std::endl;
		return;
	}
	longlongStatisticServices_[serviceName]->updateService(value);
}

void MonitorDimServer::updateErrorMessage(std::string message) {
	errorMessageService_.updateService((char*) message.data());
}

} /* namespace dim */
} /* namespace na62 */
