/*
 * ThreadTimer.h
 *
 *  Created on: Aug 25, 2016
 *      Author: ivp
 */

#ifndef SRC_THREADTIMER_H_
#define SRC_THREADTIMER_H_

#include <chrono>
#include <functional>
#include <mutex>
#include <vector>

#include "ThreadWorker.h"

namespace tmx {
namespace utils {

struct PeriodicTickData
{
	std::function<void(void)> PeriodicTick;
	std::chrono::milliseconds Frequency;
	std::chrono::milliseconds StartDelay;
	std::chrono::steady_clock::time_point LastTickTime;
};

class ThreadTimer : public ThreadWorker
{
public:
	/**
	 * Construct a new timer that runs in a separate thread.
	 *
	 * @param precision The amount of time the thread will sleep between checks to call periodic tick functions.
	 */
	explicit ThreadTimer(std::chrono::milliseconds precision = std::chrono::milliseconds(100));
	virtual ~ThreadTimer();

	/**
	 * Add a periodic tick function to call at the frequency specified.
	 *
	 * If more than 1 periodic tick functions are added, then the execution of 1 function
	 * may affect the timing of when the other function is called, since a single thread
	 * is used to call each function.
	 *
	 * @param periodicTick The function to call.
	 * @param frequency The interval that the function is called.
	 * @param startDelay The time to wait before calling the function the first time.
	 * @return An ID that can be used to later change the frequency.
	 */
	uint AddPeriodicTick(const std::function<void(void)> &periodicTick,
			std::chrono::milliseconds frequency,
			std::chrono::milliseconds startDelay = std::chrono::milliseconds(0));

	/**
	 * Change the frequency of a previously added tick function.
	 *
	 * @param id The ID of the tick function returned by AddPeroidicTick.
	 * @param frequency The new interval that the function is called.
	 */
	void ChangeFrequency(uint id, std::chrono::milliseconds frequency);

	/**
	 * Force a timer trigger event.
	*/
	void TriggerNow(uint id);

private:
	// Override of DoWork from BackgroundWorker.
	void DoWork();

	std::mutex _lock;
	std::chrono::milliseconds _precision;

	// Note that the internal code assumes that elements are never removed from this vector!
	// If a RemovePeriodicTick is ever written, a flag should just be added to PeriodicTickData
	// to show that it is no longer in use.  The index of each element must never change.
	std::vector<PeriodicTickData> _periodicTicks;
};

class ThreadTimerClient
{
public:
	explicit ThreadTimerClient(tmx::utils::ThreadTimer& timer) : _timer(timer)
	{
		_periodicTickId = _timer.AddPeriodicTick(std::bind(&ThreadTimerClient::TimerTick, this), std::chrono::milliseconds(10000));
	}
	virtual ~ThreadTimerClient() {}

	void set_Frequency(std::chrono::milliseconds frequency)
	{
		_timer.ChangeFrequency(_periodicTickId, frequency);
	}

	void TriggerNow()
	{
		_timer.TriggerNow(_periodicTickId);
	}

private:
	virtual void TimerTick() = 0;
	tmx::utils::ThreadTimer& _timer;
	uint _periodicTickId;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_THREADTIMER_H_ */
