//============================================================================
// Name        : na62-farm-control.cpp
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <socket/ZMQHandler.h>

#include "options/MyOptions.h"
#include "MonitorDimServer.h"
#include "MessageQueueConnector.h"
#include "FarmStarter.h"

using namespace na62::dim;

int main(int argc, char* argv[]) {
	/*
	 * Read program parameters
	 */
	LOG_INFO("Initializing Options");
	MyOptions::Load(argc, argv);

	char hostName[1024];
	hostName[1023] = '\0';
	if (gethostname(hostName, 1023)) {
		LOG_ERROR("Unable to get host name! Refusing to start.");
		exit(1);
	}

	na62::ZMQHandler::Initialize(1);

	MessageQueueConnector_ptr myConnector(
			new na62::dim::MessageQueueConnector());

	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	if (!MyOptions::GetBool(OPTION_IS_MERGER)) {
		strcat(hostname, "-in");
	}

	hostent * record = gethostbyname(hostname);
	if (record == NULL) {
		LOG_ERROR("Unable to determine IP of " + std::string(hostname));
		exit(1);
	}

	in_addr * address = (in_addr *) record->h_addr;
	std::string myIP = inet_ntoa(*address);

	FarmStarter starter(myConnector);

	na62::dim::MonitorDimServer_ptr dimServer_(
			new MonitorDimServer(myConnector, std::string(hostName), starter,
					myIP));
	myConnector->setDimServer(dimServer_);

	myConnector->run();

	return 0;
}
