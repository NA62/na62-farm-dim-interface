/*
 * MonitorDimServer.cpp
 *
 *  Created on: Jun 20, 2012
 *      Author: kunzejo
 */

#include "options/MyOptions.h"
#include "MessageQueueConnector.h"
#include "MonitorDimServer.h"

namespace na62 {
namespace dim {

MonitorDimServer::MonitorDimServer(
		MessageQueueConnector_ptr messageQueueConnector, std::string hostName,
		FarmStarter& farmStarter, std::string inIpAddress) :
		hostName_(hostName), cmdh(hostName, messageQueueConnector, farmStarter), initialState_(
				OFF), stateService_(std::string(hostName + "/State").data(),
				initialState_), errorMessageService_(
				std::string(hostName + "/ErrorMessage").data(), (char*) ""), inIpAddressService_(
				std::string(hostName + "/InIpAddress").data(),
				(char*) inIpAddress.data()), messageQueueConnector_(
				messageQueueConnector) {

	auto MULTI_STAT_SERVICES = Options::GetStringList(
	OPTION_MULTI_STAT_SERVICES);
	for (unsigned int i = 0; i < MULTI_STAT_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + MULTI_STAT_SERVICES[i]);
		LOG_INFO << "Starting service " << serviceName << ENDL;

		DimService_ptr ptr(new DimService(serviceName.data(), (char*) ""));
		multiStatisticServices_[MULTI_STAT_SERVICES[i]] = ptr;
	}

	auto LONGLONG_SERVICES = Options::GetStringList(OPTION_LONGLONG_SERVICES);
	longlong initialVal = 0;
	for (unsigned int i = 0; i < LONGLONG_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + LONGLONG_SERVICES[i]);
		LOG_INFO << "Starting service " << serviceName << ENDL;

		DimService_ptr ptr(new DimService(serviceName.data(), initialVal));
		longlongStatisticServices_[LONGLONG_SERVICES[i]] = ptr;
	}

	start(hostName.data());
}

MonitorDimServer::~MonitorDimServer() {
	stop();
}

void MonitorDimServer::updateState(STATE state) {
	//int state_int = (int) state;
	initialState_ = state;
	LOG_INFO << "Updating state to " << state << ENDL;
	//stateService_.updateService(state_int);
	stateService_.updateService();

	if (state == OFF) {
		for (auto serviceAndName : multiStatisticServices_) {
			serviceAndName.second->updateService((char*)"");
		}
		for (auto serviceAndName : longlongStatisticServices_) {
			serviceAndName.second->updateService(0);
		}
	}
}

void MonitorDimServer::updateStatistics(std::string serviceName,
		std::string statistics) {
	if (multiStatisticServices_.find(serviceName)
			== multiStatisticServices_.end()) {
		LOG_ERROR << "Unknown service: " << serviceName << ENDL;
		return;
	}
	multiStatisticServices_[serviceName]->updateService(
			(char*) statistics.data());
}

void MonitorDimServer::updateStatistics(std::string serviceName,
		longlong value) {
	if (longlongStatisticServices_.find(serviceName)
			== longlongStatisticServices_.end()) {
		LOG_ERROR << "Unknown service: " << serviceName << ENDL;
		return;
	}

	longlongStatisticServices_[serviceName]->updateService(value);
}

void MonitorDimServer::updateErrorMessage(std::string message) {
	errorMessageService_.updateService((char*) message.data());
}

} /* namespace dim */
} /* namespace na62 */
