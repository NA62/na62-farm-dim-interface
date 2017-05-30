/*
 * MonitorDimServer.cpp
 *
 *  Created on: Jun 20, 2012
 *      Author: kunzejo
 */

#include "options/MyOptions.h"
#include "MessageQueueConnector.h"
#include "MonitorDimServer.h"
#include <structs/EOBPackets.h>

namespace na62 {
namespace dim {

MonitorDimServer::MonitorDimServer(
		MessageQueueConnector_ptr messageQueueConnector, std::string hostName,
		FarmStarter& farmStarter, std::string inIpAddress) :
		hostName_(hostName), cmdh_(hostName, messageQueueConnector,
				farmStarter), initialState_(OFF), stateService_(
				std::string(hostName + "/State").data(), initialState_), inIpAddressService_(
				std::string(hostName + "/InIpAddress").data(),
				(char*) inIpAddress.data()), messageQueueConnector_(
				messageQueueConnector) {

	auto MULTI_STAT_SERVICES = Options::GetStringList(
	OPTION_MULTI_STAT_SERVICES);
	for (unsigned int i = 0; i < MULTI_STAT_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + MULTI_STAT_SERVICES[i]);
		LOG_INFO("Starting service " + serviceName);

		DimService_ptr ptr(new DimService(serviceName.data(), (char*) ""));
		multiStatisticServices_[MULTI_STAT_SERVICES[i]] = std::make_pair(ptr,
				"");
	}

	auto LONGLONG_SERVICES = Options::GetStringList(OPTION_LONGLONG_SERVICES);
	for (unsigned int i = 0; i < LONGLONG_SERVICES.size(); i++) {
		std::string serviceName = std::string(
				hostName + "/" + LONGLONG_SERVICES[i]);
		LOG_INFO("Starting service " + serviceName);

		longlong initialVal = 0;
		DimService_ptr ptr(new DimService(serviceName.data(), initialVal));
		longlongStatisticServices_[LONGLONG_SERVICES[i]] = std::make_pair(ptr,
				initialVal);
		ptr->updateService(
				longlongStatisticServices_[LONGLONG_SERVICES[i]].second);
	}

	bool is_merger = Options::GetBool(OPTION_IS_MERGER);
	if (!is_merger) {
		//I wont start those services on the mergers
		std::vector<std::string> EOB_STAT_SERVICES;
		EOB_STAT_SERVICES = {"EOBStatsL1", "EOBStatsL2"};
		for (unsigned int i = 0; i < EOB_STAT_SERVICES.size(); i++) {
			uint structure_size = 0;
			if (EOB_STAT_SERVICES[i] == "EOBStatsL1") {
				structure_size = sizeof(l1EOBInfo);
			} else {
				structure_size = sizeof(l2EOBInfo);
			}
			std::string serviceName = std::string("NA62/EOB/" + hostName + "/" + EOB_STAT_SERVICES[i]);
			LOG_INFO("Starting service " + serviceName);
			DimService_ptr ptr(new DimService(serviceName.data(), "C", (char*) "", structure_size));
			eobStatisticServices_[EOB_STAT_SERVICES[i]] = std::make_pair(ptr, "");
		}
	}

    LOG_INFO("Starting DIM server");
	start(hostName.data());
}

MonitorDimServer::~MonitorDimServer() {
	stop();
}

void MonitorDimServer::updateState(STATE state) {
	//int state_int = (int) state;
	initialState_ = state;
	LOG_INFO("Updating state to " + state);
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
		LOG_ERROR("Unknown service: " + serviceName);
		return;
	}
	multiStatisticServices_[serviceName].second = std::move(statistics);
	multiStatisticServices_[serviceName].first->updateService(
			(char*) multiStatisticServices_[serviceName].second.data());
}
void MonitorDimServer::updateL1EOBStatistics(std::string serviceName,
		std::string statistics) {

	if (eobStatisticServices_.find(serviceName)
			== eobStatisticServices_.end()) {
		LOG_ERROR("Unknown service: " + serviceName);
		return;
	}
	eobStatisticServices_[serviceName].second = std::move(statistics);
	eobStatisticServices_[serviceName].first->updateService(
			(void*) eobStatisticServices_[serviceName].second.data(), sizeof(l1EOBInfo));
}
void MonitorDimServer::updateL2EOBStatistics(std::string serviceName,
		std::string statistics) {

	if (eobStatisticServices_.find(serviceName)
			== eobStatisticServices_.end()) {
		LOG_ERROR("Unknown service: " + serviceName);
		return;
	}
	eobStatisticServices_[serviceName].second = std::move(statistics);
	eobStatisticServices_[serviceName].first->updateService(
			(void*) eobStatisticServices_[serviceName].second.data(), sizeof(l2EOBInfo));
}


void MonitorDimServer::updateStatistics(std::string serviceName,
		longlong value) {
	if (longlongStatisticServices_.find(serviceName)
			== longlongStatisticServices_.end()) {
		LOG_ERROR("Unknown service: " + serviceName);
		return;
	}

	longlongStatisticServices_[serviceName].second = value;
	longlongStatisticServices_[serviceName].first->updateService();
}

} /* namespace dim */
} /* namespace na62 */
