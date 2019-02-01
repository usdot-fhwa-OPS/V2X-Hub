/*
 * PerformanceTimer.h
 *
 *  Created on: Dec 5, 2014
 *      Author: ivp
 */

#ifndef PERFORMANCETIMER_H_
#define PERFORMANCETIMER_H_

#include <boost/date_time.hpp>

class PerformanceTimer
{
public:
    // Returns the current high-resolution system time in UTC.
    static boost::posix_time::ptime Now() { return boost::posix_time::microsec_clock::universal_time(); }

    // Construct and start the timer.
	PerformanceTimer() : _startTime( Now() ) {};

    // Reset the timer to the current time.
    void Reset() { _startTime = Now(); }

    // Returns the elapsed time.
    boost::posix_time::time_duration const Elapsed()
    {
        return Now() - _startTime;
    }

private:
	// The time of class construction or when last Reset().
	boost::posix_time::ptime _startTime;
};

#endif /* PERFORMANCETIMER_H_ */
