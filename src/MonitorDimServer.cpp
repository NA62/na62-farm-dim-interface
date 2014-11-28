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
				initialState_), inIpAddress_(inIpAddress), inIpAddressService_(
				std::string(hostName + "/InIpAddress").data(),
				(char*) inIpAddress_.data()), messageQueueConnector_(
				messageQueueConnector) {

	auto MULTI_STAT_SERVICES = Options::GetStringList(
	OPTION_MULTI_STAT_SERVICES);
	for (unsigned int i = 0; i < MULTI_STAT_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + MULTI_STAT_SERVICES[i]);
		LOG_INFO << "Starting service " << serviceName << ENDL;

		DimService_ptr ptr(new DimService(serviceName.data(), (char*) ""));
		multiStatisticServices_[MULTI_STAT_SERVICES[i]] = std::make_pair(ptr,
				"");
	}

	auto LONGLONG_SERVICES = Options::GetStringList(OPTION_LONGLONG_SERVICES);
	for (unsigned int i = 0; i < LONGLONG_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + LONGLONG_SERVICES[i]);
		LOG_INFO << "Starting service " << serviceName << ENDL;

		longlong initialVal = 0;
		DimService_ptr ptr(new DimService(serviceName.data(), initialVal));
		longlongStatisticServices_[LONGLONG_SERVICES[i]] = std::make_pair(ptr,
				std::move(initialVal));
		ptr->updateService(longlongStatisticServices_[LONGLONG_SERVICES[i]].second);
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
			serviceAndName.second.second = "";
			serviceAndName.second.first->updateService(
					(char*) serviceAndName.second.second.c_str());
		}
		for (auto serviceAndName : longlongStatisticServices_) {
			serviceAndName.second.second = 0;
			serviceAndName.second.first->updateService();
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
	multiStatisticServices_[serviceName].second = std::move(statistics);
	multiStatisticServices_[serviceName].first->updateService(
			(char*) multiStatisticServices_[serviceName].second.data());
}

void MonitorDimServer::updateStatistics(std::string serviceName,
		longlong value) {
	if (longlongStatisticServices_.find(serviceName)
			== longlongStatisticServices_.end()) {
		LOG_ERROR << "Unknown service: " << serviceName << ENDL;
		return;
	}

	longlongStatisticServices_[serviceName].second = value;
	longlongStatisticServices_[serviceName].first->updateService();
}

} /* namespace dim */
} /* namespace na62 */
