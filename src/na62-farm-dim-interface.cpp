//============================================================================
// Name        : na62-farm-control.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Adapter between the farm software and the run control dmi server
//============================================================================

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <vector>

#include "MonitorDimServer.h"
#include "MessageQueueConnector.h"

using namespace std;
using namespace na62::dim;

boost::asio::io_service io_service;

void handle_stop(const boost::system::error_code& error, int signal_number) {
	if (!error) {
		std::cerr << "Stopping farm control" << std::endl;
		io_service.stop();
		exit(0);
	}
}

int main() {
	char hostName[1024];
	hostName[1023] = '\0';
	if (gethostname(hostName, 1023)) {
		std::cerr << "Unable to get host name! Refusing to start." << std::endl;
		exit(1);
	}

	MessageQueueConnector_ptr myConnector(new na62::dim::MessageQueueConnector());
	na62::dim::MonitorDimServer_ptr dimServer_(new MonitorDimServer (myConnector, std::string(hostName)));
	myConnector->setDimServer(dimServer_);

	// Construct a signal set registered for process termination.
	boost::asio::signal_set signals(io_service, SIGINT, SIGTERM, SIGQUIT);

	// Start an asynchronous wait for one of the signals to occur.
	signals.async_wait(handle_stop);

	boost::thread simulatorThread(
			boost::bind(&boost::asio::io_service::run, &io_service));

	myConnector->run();

	return 0;
}
