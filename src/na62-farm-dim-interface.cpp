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
#include <unistd.h>

#include "options/Options.h"

#include "MonitorDimServer.h"
#include "MessageQueueConnector.h"
#include "FarmStarter.h"

using namespace na62::dim;

boost::asio::io_service io_service;

int main(int argc, char* argv[]) {
	/*
	 * Read program parameters
	 */
	std::cout << "Initializing Options" << std::endl;
	Options::Initialize(argc, argv);

	char hostName[1024];
	hostName[1023] = '\0';
	if (gethostname(hostName, 1023)) {
		std::cerr << "Unable to get host name! Refusing to start." << std::endl;
		exit(1);
	}

	MessageQueueConnector_ptr myConnector(
			new na62::dim::MessageQueueConnector());

	na62::dim::MonitorDimServer_ptr dimServer_(
			new MonitorDimServer(myConnector, std::string(hostName)));
	myConnector->setDimServer(dimServer_);

	FarmStarter starter;

	myConnector->run();

	return 0;
}
