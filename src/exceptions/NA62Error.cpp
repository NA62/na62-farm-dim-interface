/*
 * NA62Error.cpp
 *
 *  Created on: Nov 15, 2011
 *      Author: kunzejo
 */

#include <iostream>
#include "NA62Error.h"

namespace na62 {
namespace dim {
NA62Error::NA62Error(const std::string& message) :
		std::runtime_error(message) {
	std::cerr << message << std::endl;
}
} /* namespace dim */
} /* namespace na62 */
