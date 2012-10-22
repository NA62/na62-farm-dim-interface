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
std::string KnownMultiStatServices[] = { "DetectorData", "L1TriggerData",
		"L2TriggerData" };
std::string KnownLongLongServices[] = { "BytesToMerger", "EventsToMerger",
		"L1MTPsSent", "L1TriggersSent", "PF_PacksReceived", "PF_BytesReceived",
		"PF_PacksDroppedMaster", "PF_PacksDroppedWorker" };

MonitorDimServer::MonitorDimServer(
		MessageQueueConnector_ptr messageQueueConnector, std::string hostName, FarmStarter& farmStarter) :
		hostName_(hostName), cmdh(hostName, messageQueueConnector, farmStarter), initialState_(
				OFF), stateService_(std::string(hostName + "/State").data(),
				initialState_), messageQueueConnector_(messageQueueConnector) {

	for (unsigned int i = 0;
			i < sizeof(KnownMultiStatServices) / sizeof(std::string); i++) {
		std::string serviceName = std::string(
				hostName + "/" + KnownMultiStatServices[i]);
		mycout << "Starting service " << serviceName << std::endl;

		DimService_ptr ptr(new DimService(serviceName.data(), (char*) ""));
		multiStatisticServices_[KnownMultiStatServices[i]] = ptr;
	}

	longlong initialVal = 0;
	for (unsigned int i = 0;
			i < sizeof(KnownLongLongServices) / sizeof(std::string); i++) {
		std::string serviceName = std::string(
				hostName + "/" + KnownLongLongServices[i]);
		mycout << "Starting service " << serviceName << std::endl;

		DimService_ptr ptr(new DimService(serviceName.data(), initialVal));
		longlongStatisticServices_[KnownLongLongServices[i]] = ptr;
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

} /* namespace dim */
} /* namespace na62 */
