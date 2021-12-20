/*
 * SignalException.h
 *
 *  Created on: Oct 27, 2016
 *      Author: gmb
 */

#ifndef SRC_SIGNALEXCEPTION_H_
#define SRC_SIGNALEXCEPTION_H_

#include <csignal>
#include <string.h>
#include <tmx/TmxException.hpp>

namespace tmx
{
namespace utils
{

class SignalException: public tmx::TmxException
{
public:
	explicit SignalException(int signalNo): tmx::TmxException(strsignal(signalNo)), sigNum(signalNo) {}
	int GetSignalNumber() { return sigNum; }
private:
	int sigNum;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_SIGNALEXCEPTION_H_ */
