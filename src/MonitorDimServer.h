/*
 * MonitorDimServer.h
 *
 *  Created on: Jun 20, 2012
 *      Author: kunzejo
 */

#ifndef MONITORDIMSERVER_H_
#define MONITORDIMSERVER_H_

#include <dis.hxx>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <monitoring/IPCHandler.h>

#include "FarmStarter.h"
#include "MessageQueueConnector.h"
#include "MyCommandHandler.h"

namespace na62 {
namespace dim {
class MessageQueueConnector;
typedef boost::shared_ptr<MessageQueueConnector> MessageQueueConnector_ptr;
typedef boost::shared_ptr<DimService> DimService_ptr;

class MonitorDimServer: public boost::enable_shared_from_this<MonitorDimServer>,
		DimServer,
		private boost::noncopyable {
private:
	const std::string hostName_;
	MyCommandHandler cmdh_;

	int initialState_;
	DimService stateService_;
	DimService inIpAddressService_;
	std::map<std::string, std::pair<DimService_ptr, std::string>> multiStatisticServices_;
	std::map<std::string, std::pair<DimService_ptr, std::string>> eobStatisticServices_;
	std::map<std::string, std::pair<DimService_ptr, long long>> longlongStatisticServices_;

	MessageQueueConnector_ptr messageQueueConnector_;

public:
	MonitorDimServer(MessageQueueConnector_ptr messageQueueConnector_,
			std::string hostName, FarmStarter& farmStarter, std::string inIpAddress);

	virtual ~MonitorDimServer();

	void updateState(STATE state);
	void updateStatistics(std::string serviceName, std::string statistics);
	void updateL1EOBStatistics(std::string serviceName, std::string statistics);
	void updateL2EOBStatistics(std::string serviceName, std::string statistics);
	void updateStatistics(std::string serviceName, longlong value);
};

typedef boost::shared_ptr<MonitorDimServer> MonitorDimServer_ptr;

} /* namespace dim */
}
/* namespace na62 */
#endif /* MONITORDIMSERVER_H_ */
