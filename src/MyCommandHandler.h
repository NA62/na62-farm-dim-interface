/*
 * MyCommandHandler.h
 *
 *  Created on: Jun 21, 2012
 *      Author: kunzejo
 */

#ifndef MYCOMMANDHANDLER_H_
#define MYCOMMANDHANDLER_H_

#include <dim/dis.hxx>
#include <string>

#include "FarmStarter.h"
#include "MessageQueueConnector.h"

namespace na62 {
namespace dim {
class MyCommandHandler: public DimCommandHandler // In order to inherit "commandHandler"
{
public:
	// The constructor creates the Commands
	MyCommandHandler(std::string& hostname,
			MessageQueueConnector_ptr messageQueueConnector,
			FarmStarter& farmStarter);

	// Overloaded method commandHandler called whenever commands arrive,
	void commandHandler();

private:
	DimCommand *command;
	MessageQueueConnector_ptr messageQueueConnector_;
	FarmStarter& farmStarter_;

};

inline MyCommandHandler::MyCommandHandler(std::string& hostname,
		MessageQueueConnector_ptr messageQueueConnector,
		FarmStarter& farmStarter) :
		command(
				new DimCommand((hostname + std::string("/CMD")).data(),
						(char*) ("C"), this)), messageQueueConnector_(
				messageQueueConnector), farmStarter_(farmStarter) {
}

} /* namespace dim */
} /* namespace na62 */
#endif /* MYCOMMANDHANDLER_H_ */
