/*
 * FarmStarter.h
 *
 *  Created on: Sep 12, 2012
 *      Author: root
 */

#ifndef FARMSTARTER_H_
#define FARMSTARTER_H_

#include <dim/dic.hxx>

namespace na62 {
namespace dim {

class FarmStarter: public DimClient {
public:
	FarmStarter();
	virtual ~FarmStarter();

private:
	void startFarm(std::string param);
	void killFarm();

	void infoHandler();

	 DimInfo availableSourceIDs_;
	 pid_t farmPID_;
};
} /* namespace dim */
} /* namespace na62 */
#endif /* FARMSTARTER_H_ */
