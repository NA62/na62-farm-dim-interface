/*
 * MessageQueueConnector.h
 *
 *  Created on: Jul 11, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */
#ifndef STATE_ENUM_H_
#define STATE_ENUM_H_

namespace na62 {
namespace dim {
enum STATE {
	// 0=IDLE; 1=INITIALIZED; 2=RUNNING; Other=ERROR
	OFF,
	INITIALIZED,
	RUNNING,
	ERROR
};
}
}

#endif /* STATE_ENUM_H_ */
