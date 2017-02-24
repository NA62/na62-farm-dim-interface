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
#include <thread>

using namespace na62::dim;

void processorMonitor(FarmStarter *starter) {

	while (true) {
		int count_alive = 0;
		for (auto &processor_pid : starter->getProcessorPID()) {
			if (!kill( (int) processor_pid, 0)) {
				LOG_INFO(processor_pid<<" still alive");
				++count_alive;
			} else {
				LOG_ERROR(processor_pid<<"       dead");
			}
		}


		if (starter->getMonitoringStatus()) {
			//starter->startProcessors(1);
			//starter->startProcessors(starter->getProcessorAmount() - count_alive);

			while (count_alive++ < starter->getProcessorAmount()) {
				boost::filesystem::path exec_path(starter->getSharedProcessorPath());
				LOG_INFO("Starting trigger processor " << exec_path.string());
				std::vector<std::string> triggerProcessorParams = starter->generateStartParameters("trigger-processor");

				signal(SIGCHLD, SIG_IGN);//don't want to wait for child process created

				pid_t child_pid = fork();
				if (child_pid == 0) {
					LOG_INFO("Child: " << child_pid << " " << getpid());

					char* argv[triggerProcessorParams.size() + 2];
					argv[0] = (char*) exec_path.filename().string().data();

					for (unsigned int i = 0; i < triggerProcessorParams.size(); i++) {
						argv[i + 1] = (char*) triggerProcessorParams[i].data();
					}
					argv[triggerProcessorParams.size() + 1] = NULL;

					if (execv(exec_path.string().data(), argv) < 0) {
						LOG_ERROR("Error starting the new process" << exec_path.string());
					}
					LOG_ERROR("Error child not started!!");
					exit(0);
				}
				starter->pushPID(child_pid);
			}
		}
		/*
		* Allow other threads to run
		*/
		boost::this_thread::sleep(boost::posix_time::microsec(100));
		sleep(5);
	}
}

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

	//Dim server that receive commands
	FarmStarter starter(myConnector);


	na62::dim::MonitorDimServer_ptr dimServer_(
			new MonitorDimServer(myConnector, std::string(hostName), starter,
					myIP));
	myConnector->setDimServer(dimServer_);

	std::thread processor_monitor(processorMonitor, &starter);
	//processor_monitor.detach();

	myConnector->run();

	return 0;
}
