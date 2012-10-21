/*
 * MessageQueueConnector.cpp
 *
 *  Created on: Jul 11, 2012
 *      Author: root
 */
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "MonitorDimServer.h"
#include "States.h"
#include "options/Options.h"

#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {

using namespace boost::interprocess;

MessageQueueConnector::MessageQueueConnector() {
	// TODO Auto-generated constructor stub

}

MessageQueueConnector::~MessageQueueConnector() {
	// TODO Auto-generated destructor stub
}

void MessageQueueConnector::run() {
	while (true) {
		try {
			message_queue::remove("state");
			message_queue stateQueue(create_only, "state" //name
					, 1 // max message number
					, sizeof(int) // max message size
					);

			message_queue::remove("statistics");
			message_queue statisticsQueue(create_only, "statistics" //name
					, 100 // max message number
					, 1024 * 64 // max message size
							);
			unsigned int priority;
			message_queue::size_type recvd_size;

			STATE state = OFF;

			std::string statisticsMessage;
			while (true) {
				boost::posix_time::time_duration timeoutDuration(
						boost::posix_time::milliseconds(Options::HEARTBEAT_TIMEOUT_MILLIS));

				boost::posix_time::ptime timeout = boost::posix_time::ptime(
						boost::posix_time::second_clock::universal_time()
								+ timeoutDuration);

				if (stateQueue.timed_receive(&state, sizeof(int), recvd_size,
						priority, timeout)) {
					dimServer_->updateState(state);
					std::cerr << "Received state" << state << " : " << priority
							<< std::endl;

					while (statisticsQueue.get_num_msg() > 0) {
						statisticsMessage.resize(1024 * 64);
						if (statisticsQueue.try_receive(&(statisticsMessage[0]),
								statisticsMessage.size(), recvd_size,
								priority)) {
							statisticsMessage.resize(recvd_size);

							std::string statisticsName =
									statisticsMessage.substr(0,
											statisticsMessage.find(':'));
							std::string statistics = statisticsMessage.substr(
									statisticsMessage.find(':') + 1);

							if (statistics.find(";") != std::string::npos) {
								dimServer_->updateStatistics(statisticsName,
										statistics);
							} else {
								dimServer_->updateStatistics(statisticsName,
										boost::lexical_cast<longlong>(
												statistics));
							}

							std::cout << "Received: " << statisticsMessage
									<< std::endl;

						}
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
}

void MessageQueueConnector::sendCommand(std::string command) {
	if (!commandQueue_) {
		try {
			commandQueue_.reset(new message_queue(open_only // only create
					, "command" // name
					));
		} catch (interprocess_exception &ex) {
			commandQueue_.reset();
			std::cerr << "Unable to connect to command message queue: "
					<< ex.what() << std::endl;
			return;
		}
	}
	try {
		if (!commandQueue_->try_send(&(command[0]), command.size(), 0)) {
			std::cout << "Unable to send command to program via IPC! "
					<< std::endl;
			commandQueue_.reset();
		}
	} catch (interprocess_exception &ex) {
		std::cout << "Unable to send command to program via IPC! " << std::endl;
		commandQueue_.reset();
	}
}
} /* namespace dim */
} /* namespace na62 */
