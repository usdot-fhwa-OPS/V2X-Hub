/*
 * PluginMonitor.h
 *
 *  Created on: Jul 15, 2014
 *      Author: ivp
 */

#ifndef PLUGINMONITOR_H_
#define PLUGINMONITOR_H_

#include "Plugin.h"
#include <boost/thread.hpp>
#include <boost/process.hpp>
#include <pthread.h>
#include <map>

/**
 * PluginMonitor
 * \ingroup IVPCore
 */
class PluginMonitor : public Plugin
{
public:
	PluginMonitor(MessageRouter *messageRouter);
	~PluginMonitor();

	virtual void onMessageReceived(IvpMessage *msg);

protected:

private:
	boost::thread mMonitorThread;

	enum PluginKillState {
		PluginKillState_alive,
		PluginKillState_sigTermSent,
		PluginKillState_sigIntSent,
		PluginKillState_sigKillSent
	};

	struct MessageData
	{
		uint64_t firstReceivedTime;
		uint64_t lastReceivedTime;

		MessageData()
		{
			firstReceivedTime = 0;
			lastReceivedTime = 0;
		}
	};

	struct PluginMonitoringData {
		pid_t pid;
		uint64_t startTime;
		uint64_t lastReceivedTime;
		//uint64_t lastKillSignalTime;
		PluginKillState killStatus;

		PluginMonitoringData() {
			pid = 0;
			startTime = 0;
			lastReceivedTime = 0;
			//lastKillSignalTime = 0;
			killStatus = PluginKillState_alive;
		}
	};

	std::map<unsigned int, MessageData> mMessages;
	pthread_mutex_t mMessagesLock;

	std::map<InstalledPluginEntry, PluginMonitoringData> mRunningPlugins;

	static std::vector<gid_t> sPluginGids;
	static uid_t sPluginUid;

	void monitorThreadEntry();

	static bool procSetupCloseAllFd(int fd);
	static void procSetupBeforeExec(boost::process::executor &e);

};

#endif /* PLUGINMONITOR_H_ */
