/*
 * HistoryManager.cpp
 *
 *  Created on: Jul 15, 2014
 *      Author: ivp
 */

#ifndef __CYGWIN__
#include <sys/prctl.h>
#endif
#include "HistoryManager.h"
#include <iostream>
#include <assert.h>
#include <string.h>
#include "version.h"
#include "logger.h"
#include <signal.h>
#include "utils/StringUtils.h"
#include "utils/TimeUtils.h"
#include <sstream>

#define HISTORYPURGER_CONFIGKEY_PURGEINTERVAL "Purge Intervals (sec)"
#define HISTORYPURGER_CONFIGKEY_MAXEVENTLOGSIZE "Max Event Log Size"

#define HISTORYPURGER_CONFIGDFLT_PURGEINTERVAL 120


using namespace std;

HistoryManager::HistoryManager(MessageRouter *messageRouter) : Plugin(messageRouter)
{
	RegistrationInformation info;
	info.pluginInfo.name = "ivpcore.HistoryManager";
	info.pluginInfo.description = "Core element that is responsible for purging old log and history data";
	info.pluginInfo.version = IVPCORE_VERSION;

	ostringstream m = ostringstream() << HISTORYPURGER_CONFIGDFLT_PURGEINTERVAL;
	info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(HISTORYPURGER_CONFIGKEY_PURGEINTERVAL,
			static_cast<ostringstream*>( &m )->str(),
			"Interval between purges of history items"));
	info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(HISTORYPURGER_CONFIGKEY_MAXEVENTLOGSIZE, "2000", "Maximum number of event log entries to keep.  A value of zero will result in no purging of event log entries"));

	try {
		this->registerPlugin(info);
	} catch (PluginException &e) {
		LOG_FATAL(e.what());
		throw e;
	}

	this->mPurgeThread = boost::thread(&HistoryManager::purgeThreadEntry, this);
}

HistoryManager::~HistoryManager()
{

}

void HistoryManager::purgeThreadEntry()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "HistoryMngr", 0, 0, 0);
#endif

	boost::this_thread::disable_interruption di;
	while(!boost::this_thread::interruption_requested())
	{
		{
			uint64_t startTime = TimeUtils::getSystemMillis();
			uint64_t sleepTime;
			do {
				sleep(1);
				char *tailptr;
				sleepTime = strtoull(this->getConfigValue(HISTORYPURGER_CONFIGKEY_PURGEINTERVAL).c_str(), &tailptr, 10);
			} while(TimeUtils::getSystemMillis() < startTime + (sleepTime == 0 ? HISTORYPURGER_CONFIGDFLT_PURGEINTERVAL : sleepTime) * 1000);
		}
		{
			char *tailptr;
			unsigned int maxEventLogSize = strtoul(this->getConfigValue(HISTORYPURGER_CONFIGKEY_MAXEVENTLOGSIZE).c_str(), &tailptr, 10);

			if (maxEventLogSize != 0)
			{
				try {
					LogContext lcontext;
					int numberPurged = lcontext.purgeOldLogEntries(maxEventLogSize);
					if (numberPurged != 0)
						LOG_DEBUG("Purged " << numberPurged << " event log entries");
				} catch (const DbException &e) {
					LOG_WARN(e.what());
				}
			}
		}
	}
}
