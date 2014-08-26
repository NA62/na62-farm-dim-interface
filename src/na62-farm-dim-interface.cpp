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

#include "options/MyOptions.h"
#include "MonitorDimServer.h"
#include "MessageQueueConnector.h"
#include "FarmStarter.h"

using namespace na62::dim;

int main(int argc, char* argv[]) {
	/*
	 * Read program parameters
	 */
	std::cout << "Initializing Options" << std::endl;
	MyOptions::Load(argc, argv);

	char hostName[1024];
	hostName[1023] = '\0';
	if (gethostname(hostName, 1023)) {
		std::cerr << "Unable to get host name! Refusing to start.";
		exit(1);
	}

	MessageQueueConnector_ptr myConnector(
			new na62::dim::MessageQueueConnector());

	FarmStarter starter(myConnector);

	na62::dim::MonitorDimServer_ptr dimServer_(
			new MonitorDimServer(myConnector, std::string(hostName), starter));
	myConnector->setDimServer(dimServer_);

	myConnector->run();

	return 0;
}
