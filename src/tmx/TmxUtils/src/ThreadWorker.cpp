/*
 * ThreadWorker.cpp
 *
 *  Created on: Aug 19, 2016
 *      Author: ivp
 */

#include "ThreadWorker.h"

using namespace std;

namespace tmx {
namespace utils {

ThreadWorker::ThreadWorker()
{
}

ThreadWorker::~ThreadWorker()
{
	Stop();
}

void ThreadWorker::Start()
{
	Stop();
	_stopThread = false;
	_thread = new thread(&ThreadWorker::DoWork, this);
}

void ThreadWorker::Stop()
{
	if (_thread != NULL && !_stopThread)
	{
		_stopThread = true;
		_thread->join();
		delete _thread;
		_thread = NULL;
	}
}

bool ThreadWorker::IsRunning()
{
	return !_stopThread && _thread != NULL;
}

} /* namespace utils */
} /* namespace tmx */
