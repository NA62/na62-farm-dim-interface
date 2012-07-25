/*
 * MessageQueueConnector.cpp
 *
 *  Created on: Jul 11, 2012
 *      Author: root
 */
#include <boost/asio.hpp>

#include "MonitorDimServer.h"
#include "States.h"

#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {

using namespace boost::interprocess;

MessageQueueConnector::MessageQueueConnector(){
	// TODO Auto-generated constructor stub

}

MessageQueueConnector::~MessageQueueConnector() {
	// TODO Auto-generated destructor stub
}

void MessageQueueConnector::run() {
	try {
		// Erase previous message queue
		message_queue::remove("state");
		// Open a message queue.
		message_queue stateQueue(create_only //only create
				, "state" //name
				, 1 // max message number
				, sizeof(int) //max message size
				);

		message_queue::remove("statistics");
		message_queue statisticsQueue(create_only //only create
				, "statistics" //name
				, 1 // max message number
				, 1024 * 64 //max message size
						);
		unsigned int priority;
		message_queue::size_type recvd_size;

		STATE state = OFF;
		std::string statistics;
		while (true) {
			statistics.resize(1024 * 64);
			boost::posix_time::time_duration timeoutDuration(
					boost::posix_time::seconds(2));

			boost::posix_time::ptime timeout = boost::posix_time::ptime(
					boost::posix_time::second_clock::universal_time()
							+ timeoutDuration);

			if (stateQueue.timed_receive(&state, sizeof(int), recvd_size,
					priority, timeout)) {
				dimServer_->updateState(state);
				std::cerr << "Received state" << state << " : " << priority
						<< std::endl;

				if (statisticsQueue.try_receive(&(statistics[0]),
						statistics.size(), recvd_size, priority)) {
					statistics.resize(recvd_size);
					std::cerr << statistics << std::endl;

					dimServer_->updateStatistics(statistics);

				} else {
					std::cerr << "Didn't receive any new statistics!"
							<< std::endl;
				}
			} else {
				std::cerr << "Timeout" << std::endl;
			}
		}

		dimServer_->updateState(OFF);
		message_queue::remove("state");
		std::cerr << "done" << std::endl;

	} catch (interprocess_exception &ex) {
		message_queue::remove("state");
		std::cerr << "Unable to connect to message queue: " << ex.what()
				<< std::endl;
		boost::system::error_code noError;
	}
}

void MessageQueueConnector::sendCommand(std::string command){
	if(!commandQueue_){
		try {
			commandQueue_.reset(new message_queue(open_only // only create
						, "command" // name
						));
			} catch (interprocess_exception &ex) {
				commandQueue_.reset();
				std::cout << "Unable to connect to command message queue: " << ex.what() << std::endl;
			}
	}
	commandQueue_->send(&(command[0]), command.size(), 0);
}

} /* namespace dim */
} /* namespace na62 */
