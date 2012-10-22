/*
 * BrokenPacketReceivedError.h
 *
 *  Created on: Nov 15, 2011
 *      Author: kunzejo
 */

#ifndef BROKENPACKETRECEIVEDERROR_H_
#define BROKENPACKETRECEIVEDERROR_H_

#include "NA62Error.h"

namespace na62 {
namespace dim {
class BrokenPacketReceivedError: public NA62Error {
public:
	BrokenPacketReceivedError(const std::string& message) :
			NA62Error(message) {
	}
};
} //namespace dim
} //namespace na62
#endif /* BROKENPACKETRECEIVEDERROR_H_ */
