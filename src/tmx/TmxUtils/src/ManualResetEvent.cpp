/*
 * ManualResetEvent.cpp
 *
 *  Created on: Dec 8, 2015
 *      Author: ivp
 */

#include "ManualResetEvent.h"

// This .cpp used to force compilation of header file.
// All implementation is in header.

namespace tmx {
namespace utils {

ManualResetEvent::ManualResetEvent() : _signalled(false)
{

}

void ManualResetEvent::Reset()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_signalled = false;
}

void ManualResetEvent::Set()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_signalled = true;
	_cv.notify_all();
}

void ManualResetEvent::WaitOne()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_cv.wait(lock, [&]{return _signalled == true;});
}

}} // namespace tmx::utils
