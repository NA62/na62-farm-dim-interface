/*
 * MessageHandler.cpp
 *
 *  Created on: Oct 22, 2012
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "MessageHandler.h"
#include "../options/Options.h"
#include <streambuf>

namespace na62 {
namespace dim {

int MessageHandler::overflow(int c) {
	if (!ignoreVerbose_ && !Options::VERBOSE) {
		return EOF;
	}

	if (c != EOF) {
		// and write the character to the standard output
		if (putc(c, stream_) == EOF) {
			return EOF;
		}
	}
	return c;
}

MessageHandler mH(stdout, false);
MessageHandler eH(stderr, true);
// initialize output streams with the output buffers
std::ostream mycout(&mH);
std::ostream mycerr(&eH);

} /* namespace dim */
} /* namespace na62 */
