/*
 * ThreadTimer.cpp
 *
 *  Created on: Aug 25, 2016
 *      Author: ivp
 */

#include "ThreadTimer.h"
#include <chrono>

#include "PluginLog.h"
#include <tmx/TmxException.hpp>

using namespace std;
using namespace std::chrono;

namespace tmx {
namespace utils {

ThreadTimer::ThreadTimer(std::chrono::milliseconds precision) :
	_precision(precision)
{
}

ThreadTimer::~ThreadTimer()
{
}

uint ThreadTimer::AddPeriodicTick(const std::function<void(void)> &periodicTick,
		std::chrono::milliseconds frequency, std::chrono::milliseconds startDelay)
{
	lock_guard<mutex> lock(_lock);

	PeriodicTickData tick;
	tick.PeriodicTick = periodicTick;
	tick.Frequency = frequency;
	tick.StartDelay = startDelay;

	_periodicTicks.push_back(tick);

	return _periodicTicks.size() - 1;
}

void ThreadTimer::ChangeFrequency(uint id, std::chrono::milliseconds frequency)
{
	lock_guard<mutex> lock(_lock);

	if (id >= _periodicTicks.size())
	{
		PLOG(logERROR) << "ChangeFrequency: Invalid ID.";
		BOOST_THROW_EXCEPTION(tmx::TmxException("ChangeFrequency: Invalid ID"));
	}

	_periodicTicks[id].Frequency = frequency;
}

void ThreadTimer::TriggerNow(uint id)
{
	lock_guard<mutex> lock(_lock);

	if (id >= _periodicTicks.size())
	{
		PLOG(logERROR) << "ChangeFrequency: Invalid ID.";
		BOOST_THROW_EXCEPTION(tmx::TmxException("ChangeFrequency: Invalid ID"));
	}
	_periodicTicks[id].LastTickTime = steady_clock::now() - _periodicTicks[id].Frequency;

//	if (!_stopThread)
//	{
//		for (uint index = 0; true; index++)
//		{
//			// Get a copy of the data for the current function to call.
//			// A copy is obtained so that the lock is not held while the PeriodicTick function is called.
//			PeriodicTickData tick;
//			{
//				lock_guard<mutex> lock(_lock);
//
//				if (index >= _periodicTicks.size())
//					break;
//
//				tick = _periodicTicks[index];
//			}
//
//			// Call the function.
//			tick.PeriodicTick();
//		}
//	}
}

void ThreadTimer::DoWork()
{
	steady_clock::time_point startTime = steady_clock::now();

	{
		lock_guard<mutex> lock(_lock);

		for (PeriodicTickData &tick: _periodicTicks)
		{
			tick.LastTickTime = startTime - tick.Frequency + tick.StartDelay;
		}
	}

	while (!_stopThread)
	{
		for (uint index = 0; true; index++)
		{
			// Get a copy of the data for the current function to call.
			// A copy is obtained so that the lock is not held while the PeriodicTick function is called.
			PeriodicTickData tick;
			{
				lock_guard<mutex> lock(_lock);

				if (index >= _periodicTicks.size())
					break;

				tick = _periodicTicks[index];
			}

			// Call the function if the frequency has arrived.

			steady_clock::time_point now = steady_clock::now();

			milliseconds period = duration_cast<milliseconds> (now - tick.LastTickTime);

			if (tick.Frequency > milliseconds(0) && period >= tick.Frequency)
			{
				// Call the function.
				tick.PeriodicTick();

				// Update the last time the function was called.
				// Assume that elements are never removed from the _periodicTicks vector and that
				// they are never rearranged (i.e. index still refers to the same element).
				{
					lock_guard<mutex> lock(_lock);
					_periodicTicks[index].LastTickTime = now;
				}
			}
		}

		std::this_thread::sleep_for(_precision);
	}
}

} /* namespace utils */
} /* namespace tmx */
