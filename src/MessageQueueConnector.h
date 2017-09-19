/*
 * MessageQueueConnector.h
 *
 *  Created on: Jul 11, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef MESSAGEQUEUECONNECTOR_H_
#define MESSAGEQUEUECONNECTOR_H_

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <monitoring/IPCHandler.h>
#include <string>

namespace na62 {
namespace dim {

class MonitorDimServer;
typedef boost::shared_ptr<MonitorDimServer> MonitorDimServer_ptr;

class MessageQueueConnector: private boost::noncopyable {
public:
	MessageQueueConnector();
	virtual ~MessageQueueConnector();

	void run();
	void sendCommand(std::string command);

	void setDimServer(const na62::dim::MonitorDimServer_ptr dimServer) {
		dimServer_ = dimServer;
	}

	void sendState(STATE state);

private:
	MonitorDimServer_ptr dimServer_;
	STATE lastSentState_;
};

typedef boost::shared_ptr<MessageQueueConnector> MessageQueueConnector_ptr;

} /* namespace dim */
} /* namespace na62 */
#endif /* MESSAGEQUEUECONNECTOR_H_ */
