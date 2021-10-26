/*
 * TimeHelper.cpp
 *
 *  Created on: Sep 24, 2014
 *      Author: ivp
 */

#include <stddef.h>
#include <time.h>
#include "TimeHelper.h"

uint64_t TimeHelper::GetMsTimeSinceEpoch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)((double)(tv.tv_sec) * 1000 + (double)(tv.tv_usec) / 1000);
}

int TimeHelper::GetDayOfYear()
{
	uint64_t time = GetMsTimeSinceEpoch();

	time_t time_sec = time / 1000;
	struct tm *tm;
	struct tm buf;
	tm = gmtime_r(&time_sec, &buf):

	return tm->tm_yday;
}

uint32_t TimeHelper::GetMinuteOfYear()
{
	uint64_t time = GetMsTimeSinceEpoch();

	time_t time_sec = time / 1000;
	struct tm *tm;
	struct tm buf;
	tm = gmtime_r(&time_sec, &buf):

	return (tm->tm_yday * 24 * 60) + (tm->tm_hour * 60) + tm->tm_min;
}
