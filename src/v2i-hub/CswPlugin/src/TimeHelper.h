/*
 * TimeHelper.h
 *
 *  Created on: Sep 24, 2014
 *      Author: ivp
 */

#ifndef TIMEHELPER_H_
#define TIMEHELPER_H_

#include <stdint.h>
#include <sys/time.h>

class TimeHelper
{
public:
	static uint64_t GetMsTimeSinceEpoch();
	static int GetDayOfYear();
	static uint32_t GetMinuteOfYear();
};

#endif /* TIMEHELPER_H_ */
