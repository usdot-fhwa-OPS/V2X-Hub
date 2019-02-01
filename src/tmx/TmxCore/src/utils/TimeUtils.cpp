/*
 * TimeUtils.cpp
 *
 *  Created on: Aug 2, 2014
 *      Author: ivp
 */

#include "TimeUtils.h"
#include <time.h>


uint64_t TimeUtils::getSystemMillis()
{
	timespec time;
	clock_gettime(CLOCK_REALTIME, &time);

	return (uint64_t)time.tv_sec * 1000 + (uint64_t)time.tv_nsec / 1000000;
}
