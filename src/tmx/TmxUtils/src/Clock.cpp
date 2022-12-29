/*
 * Clock.cpp
 *
 *  Created on: Aug 30, 2016
 *      Author: ivp
 */

#include "Clock.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace std::chrono;

namespace tmx {
namespace utils {

template <typename C>
static string GetChronoClockInfo()
{
	ostringstream ss;

	ss << "\tPrecision: ";

	// If time unit is less than or equal to one millisecond.

	// Type of the time unit.
	typedef typename C::period P;
	if (ratio_less_equal<P,milli>::value)
	{
		// Convert to milliseconds.
		typedef typename ratio_multiply<P,kilo>::type TT;
		ss << fixed << double(TT::num)/TT::den << " milliseconds" << endl;
	}
	else
	{
		// Print as seconds.
		ss << fixed << double(P::num)/P::den << " seconds" << endl;
	}

	ss << "\tIs Steady: " << boolalpha << C::is_steady << endl;

	return ss.str();
}

void Clock::PrintInfoForChronoClocks()
{
	chrono::system_clock::time_point tp;
	cout << "system_clock" << endl;
	cout << GetChronoClockInfo<chrono::system_clock>();
	cout << "\tEpoch Local: " << ToLocalTimeString(tp) << endl;
	cout << "\tEpoch UTC:   " << ToUtcTimeString(tp) << endl;
	tp = chrono::system_clock::time_point::max();
	cout << "\tMax   UTC:   " << ToUtcTimeString(tp) << endl;
	tp = chrono::system_clock::now();
	cout << "\tNow Local: " << ToLocalTimeString(tp) << endl;
	cout << "\tNow UTC:   " << ToUtcTimeString(tp) << endl;

	cout << "high_resolution_clock" << endl;
	cout << GetChronoClockInfo<std::chrono::high_resolution_clock>();

	cout << "steady_clock" << endl;
	cout << GetChronoClockInfo<std::chrono::steady_clock>();
}

std::string Clock::ToLocalTimeString(const std::chrono::system_clock::time_point& tp)
{
    // Convert to system time.
    std::time_t t = chrono::system_clock::to_time_t(tp);
    // Convert to calendar time string.
    // Note: could have also called std:ctime(&t) - it's an alias.
	struct tm tm;
	localtime_r(&t, &tm);
	std::array<char, 20> tmBuffer;
	strftime(tmBuffer.data(), 20, "%F %T", &tm);
    std::string calStr = tmBuffer.data();
    // Remove trailing newline.
    calStr.resize(calStr.size()-1);
    return calStr;
}

std::string Clock::ToUtcTimeString(const std::chrono::system_clock::time_point& tp)
{
    // Convert to system time.
    std::time_t t = chrono::system_clock::to_time_t(tp);
    // Convert to calendar time string.
	struct tm tm;
	gmtime_r(&t, &tm);
	std::array<char, 20> tmBuffer;
	strftime(tmBuffer.data(), 20, "%F %T", &tm);
    std::string calStr = tmBuffer.data();
    // Remove trailing newline.
    calStr.resize(calStr.size()-1);
    return calStr;
}


std::string Clock::ToLocalPreciseTimeString(const std::chrono::system_clock::time_point& tp)
{
    std::time_t t = chrono::system_clock::to_time_t(tp);
	short ms = tp.time_since_epoch() / std::chrono::milliseconds(1) % 1000;
	struct tm myTm;
	localtime_r(&t, &myTm);
	char tmBuffer[20];
	strftime(tmBuffer, 20, "%F %T", &myTm);

	ostringstream ss;
    ss << tmBuffer << "." << std::setfill('0') << std::setw(3) << ms;
    return ss.str();
}

std::string Clock::ToUtcPreciseTimeString(const std::chrono::system_clock::time_point& tp)
{
    std::time_t t = chrono::system_clock::to_time_t(tp);
	short ms = tp.time_since_epoch() / std::chrono::milliseconds(1) % 1000;
	struct tm myTm;
	gmtime_r(&t, &myTm);
	char tmBuffer[20];
	strftime(tmBuffer, 20, "%F %T", &myTm);

	ostringstream ss;
    ss << tmBuffer << "." << std::setfill('0') << std::setw(3) << ms;
    return ss.str();
}


std::string Clock::ToUtcPreciseTimeString(uint64_t ms)
{
	// Convert time_t struct into UTC timestamp string.
	std::time_t t = ms/1000;
	short msec = ms%1000;
	struct tm myTm;
	gmtime_r(&t, &myTm);
	char tmBuffer[20];
	strftime(tmBuffer, 20, "%F %T", &myTm);

	ostringstream ss;
    ss << tmBuffer << "." << std::setfill('0') << std::setw(3) << msec;
    return ss.str();
}

long Clock::getAdjustedTime(unsigned int offset_tenthofSec)
{
	typedef duration<int, ratio_multiply<hours::period, ratio<24> >::type> days;

	int offset_ms = offset_tenthofSec * 100;
	system_clock::time_point nowTimePoint = system_clock::now();
	system_clock::time_point nowPlusOffsetTimePoint = nowTimePoint + milliseconds(offset_ms);


	system_clock::duration tp = nowPlusOffsetTimePoint.time_since_epoch();

	days d = duration_cast<days>(tp);
	tp -= d;

	hours h = duration_cast<hours>(tp);
	tp -= h;

	minutes m = duration_cast<minutes>(tp);
	tp -= m;

	seconds s = duration_cast<seconds>(tp);
	tp -= s;

	milliseconds ms = duration_cast<milliseconds>(tp);

	double fractionSeconds = s.count() + (ms.count()/1000.0);
	double retTimeD = ((m.count() * 60) + fractionSeconds) * 10;

	return (long)retTimeD;
}

std::string Clock::ToTimeString(const std::chrono::milliseconds ms)
{
	// Split into hours, minutes, seconds, and milliseconds.
	hours   hh = duration_cast<hours>(ms);
	minutes mm = duration_cast<minutes>(ms % chrono::hours(1));
	seconds ss = duration_cast<seconds>(ms % chrono::minutes(1));
	milliseconds msec = duration_cast<milliseconds>(ms % chrono::seconds(1));

	ostringstream oss;

	oss << setfill('0')
		<< setw(2) << hh.count() << ":"
		<< setw(2) << mm.count() << ":"
		<< setw(2) << ss.count() << "."
		<< setw(3) << msec.count();

	return oss.str();
}

uint64_t Clock::GetMillisecondsSinceEpoch()
{
	auto now = std::chrono::system_clock::now();
	return Clock::GetMillisecondsSinceEpoch(now);
}

uint64_t Clock::GetMillisecondsSinceEpoch(const std::chrono::system_clock::time_point &tp)
{
	return std::chrono::duration_cast<chrono::milliseconds>(tp.time_since_epoch()).count();
}

uint32_t Clock::GetDayOfYear() {
	system_clock::time_point now = system_clock::now();	
	time_t tt = system_clock::to_time_t(now);
	tm utc_tm;
	gmtime_r(&tt, &utc_tm);
	return utc_tm.tm_yday;
}

uint32_t Clock::GetMinuteOfYear() {
	system_clock::time_point now = system_clock::now();	
	time_t tt = system_clock::to_time_t(now);
	tm utc_tm;
	gmtime_r(&tt, &utc_tm);
	return (utc_tm.tm_yday * 24 * 60) + (utc_tm.tm_hour * 60) + utc_tm.tm_min;
}


} /* namespace utils */
} /* namespace tmx */
