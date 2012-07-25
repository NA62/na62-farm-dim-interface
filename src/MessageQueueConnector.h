/*
 * MessageQueueConnector.h
 *
 *  Created on: Jul 11, 2012
 *      Author: root
 */

#ifndef MESSAGEQUEUECONNECTOR_H_
#define MESSAGEQUEUECONNECTOR_H_

#include <boost/noncopyable.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

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

private:
	MonitorDimServer_ptr dimServer_;

	typedef boost::shared_ptr<boost::interprocess::message_queue> message_queue_ptr;
	message_queue_ptr commandQueue_;
};

typedef boost::shared_ptr<MessageQueueConnector> MessageQueueConnector_ptr;

} /* namespace dim */
} /* namespace na62 */
#endif /* MESSAGEQUEUECONNECTOR_H_ */
