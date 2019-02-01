/*
 * AutoResetEvent.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: ivp
 */

#include "AutoResetEvent.h"

AutoResetEvent::AutoResetEvent(bool initial)
	: _flag(initial)
{
	pthread_mutex_init(&_protect, NULL);
	pthread_cond_init(&_signal, NULL);
}

void AutoResetEvent::Set()
{
	pthread_mutex_lock(&_protect);
	_flag = true;
	pthread_mutex_unlock(&_protect);
	pthread_cond_signal(&_signal);
}

void AutoResetEvent::Reset()
{
	pthread_mutex_lock(&_protect);
	_flag = false;
	pthread_mutex_unlock(&_protect);
}

bool AutoResetEvent::WaitOne()
{
	pthread_mutex_lock(&_protect);
	while (!_flag) // prevent spurious wakeups from doing harm
		pthread_cond_wait(&_signal, &_protect);
	_flag = false; // waiting resets the flag
	pthread_mutex_unlock(&_protect);
	return true;
}

AutoResetEvent::~AutoResetEvent()
{
	pthread_mutex_destroy(&_protect);
	pthread_cond_destroy(&_signal);
}
