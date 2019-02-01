/*
 * HistoryManager.h
 *
 *  Created on: Jul 15, 2014
 *      Author: ivp
 */

#ifndef HISTORYMANAGER_H_
#define HISTORYMANAGER_H_

#include "Plugin.h"
#include <boost/thread.hpp>
#include <boost/process.hpp>
#include <pthread.h>
#include <map>

/**
 * Keeps a rolling history for messages sent by plugins in the IVP System.
 * \ingroup IVPCore
 */
class HistoryManager : public Plugin
{
public:
	HistoryManager(MessageRouter *messageRouter);
	~HistoryManager();

protected:

private:
	boost::thread mPurgeThread;

	void purgeThreadEntry();

};

#endif /* HISTORYMANAGER_H_ */
