/*
 * ManualResetEvent.h
 *
 *  Created on: Dec 8, 2015
 *      Author: ivp
 */

#ifndef SRC_MANUALRESETEVENT_H_
#define SRC_MANUALRESETEVENT_H_

#include <atomic>
#include <chrono>
#include <condition_variable>

namespace tmx {
namespace utils {

class ManualResetEvent
{
public:
	ManualResetEvent();
	ManualResetEvent(const ManualResetEvent& other) = delete;

	// Reset this event to not signaled.
	void Reset();

	// Signal this event.
	void Set();

	// Block the current thread until this event is signaled.
	void WaitOne();

	// Block the current thread until this event is signaled
	// or the timeout expires.
	// Returns true if the event was signaled; false if timeout expired.
	template<typename Rep, typename Period>
	bool WaitOne(const std::chrono::duration<Rep, Period>& timeout)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return _cv.wait_for(lock, timeout, [&]{return _signalled == true;});
	}

private:
	std::condition_variable _cv;
	std::mutex _mutex;
	std::atomic_bool _signalled;
};

}} // namespace tmx::utils

#endif /* SRC_MANUALRESETEVENT_H_ */
