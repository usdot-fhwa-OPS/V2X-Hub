/*
 * AutoResetEvent.h
 *
 *  Created on: Dec 5, 2014
 *      Author: ivp
 */

#ifndef AUTORESETEVENT_H_
#define AUTORESETEVENT_H_

#include <pthread.h>
#include <stdio.h>

class AutoResetEvent
{
public:
	explicit AutoResetEvent(bool initial = false);

	~AutoResetEvent();
	void Set();
	void Reset();
	bool WaitOne();

private:
	AutoResetEvent(const AutoResetEvent&);
	AutoResetEvent& operator=(const AutoResetEvent&); // non-copyable
	bool _flag;
	pthread_mutex_t _protect;
	pthread_cond_t _signal;
};


#endif /* AUTORESETEVENT_H_ */
