/*
 * PluginMonitor.cpp
 *
 *  Created on: Jul 15, 2014
 *      Author: ivp
 */

#ifndef __CYGWIN__
#include <sys/prctl.h>
#endif
#include "PluginMonitor.h"
#include <iostream>
#include <assert.h>
#include <string.h>
#include "version.h"
#include "logger.h"
#include <signal.h>
#include "utils/StringUtils.h"
#include "utils/TimeUtils.h"
#include <sstream>
#include <sys/wait.h>

#include <pwd.h>
#include <grp.h>

#define PLUGIN_USER "plugin"
#define INVALID_UID -1

#define PLUGINMONITOR_CONFIGKEY_STARTUPDELAY_MS "Startup Delay (ms)"
#define PLUGINMONITOR_CONFIGKEY_CHECKINTERVAL_MS "Monitor Check Interval (ms)"
#define PLUGINMONITOR_CONFIGKEY_MAXSTARTUPTIME_MS "Max Startup Time (ms)"

#define PLUGINMONITOR_STATUSKEY_NUMBERPLUGINS "Number of Active Plugins"

using namespace std;

uid_t PluginMonitor::sPluginUid = 0;
vector<gid_t> PluginMonitor::sPluginGids;

PluginMonitor::PluginMonitor(MessageRouter *messageRouter) : Plugin(messageRouter)
{
	RegistrationInformation info;
	info.pluginInfo.name = "ivpcore.PluginMonitor";
	info.pluginInfo.description = "Core element that is responsible for starting/stopping installed plugins and monitoring the status of the plugins";
	info.pluginInfo.version = IVPCORE_VERSION;

	info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(PLUGINMONITOR_CONFIGKEY_STARTUPDELAY_MS, "10000", "Delay in milliseconds before starting any plugins."));
	info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(PLUGINMONITOR_CONFIGKEY_CHECKINTERVAL_MS, "5000", "Delay in milliseconds between monitor checks."));
	info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(PLUGINMONITOR_CONFIGKEY_MAXSTARTUPTIME_MS, "15000", "Maximum allowed startup time of a plugin before it is rebooted."));

	try {
		this->registerPlugin(info);
	} catch (PluginException &e) {
		LOG_FATAL(e.what());
		throw e;
	}

	// Register for all messages.
	vector<MessageFilterEntry> entries;
	MessageFilterEntry entry;
	entry.type = "*";
	entries.push_back(entry);
	this->subscribeForMessages(entries);

	map<string, string> statusItems;
	statusItems[PLUGINMONITOR_STATUSKEY_NUMBERPLUGINS] = "0";
	this->setStatusItems(statusItems);

	pthread_mutex_init(&this->mMessagesLock, NULL);

	struct passwd *pwd = NULL;
	struct passwd *result = nullptr;
    	std::array<char, 64> buf; //assuming passwords are 64 bits long most
    	memset(&buf, 0, sizeof buf);
	if (getuid() != 0)
	{
		this->setPluginStatus("ERROR: Not running as root");
		this->addEventLogEntry(LogLevel_Fatal, "Error starting: Not running as root");
	}


	else if( getpwnam_r(PLUGIN_USER, pwd, buf, 64, &result) == NULL) 
	{
		this->setPluginStatus("ERROR: User '" PLUGIN_USER "' doesn't exist");
		this->addEventLogEntry(LogLevel_Fatal, "Error starting: User '" PLUGIN_USER "' doesn't exist");
	}
	else
	{
		PluginMonitor::sPluginGids.clear();
		
		char *buffer = NULL;
		size_t buffer_len = 0;
		struct group grp = { nullptr, };
		struct group *gid= nullptr;
		
		getgrnam_r("dialout", &grp, buffer, buffer_len, &gid);

		if (gid != NULL)
			PluginMonitor::sPluginGids.push_back(gid->gr_gid);

		PluginMonitor::sPluginUid = (*result).pw_uid;

		this->mMonitorThread = boost::thread(&PluginMonitor::monitorThreadEntry, this);

		this->setPluginStatus("Delayed Start...");
	}
}

PluginMonitor::~PluginMonitor()
{

}

// Since PluginMonitor is subscribed to all messages, this method will be called for every message.
// This method is executed on whatever thread the message was received on.
// For messages from external plugins, this will be called from the processor thread
// of PluginConnection.
void PluginMonitor::onMessageReceived(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg->sourceId == 0)
		return;

	uint64_t rxTime = TimeUtils::getSystemMillis();

	// Store the timing information for the message into the mMessages map.
	// It will be used by the thread to start/stop plugins as needed.

	pthread_mutex_lock(&this->mMessagesLock);

	map<unsigned int, MessageData>::iterator entry = this->mMessages.find(msg->sourceId);
	if (entry == this->mMessages.end())
	{
		// This is a new entry, add it to the map.
		MessageData messageData;
		messageData.firstReceivedTime = rxTime;
		messageData.lastReceivedTime = rxTime;
		this->mMessages[msg->sourceId] = messageData;
	}
	else
	{
		// This is an existing entry, just update the last received time.
		entry->second.lastReceivedTime = rxTime;
	}

	pthread_mutex_unlock(&this->mMessagesLock);
}

bool PluginMonitor::procSetupCloseAllFd(int fd)
{
	return fd != STDOUT_FILENO && fd != STDERR_FILENO;
}

void PluginMonitor::procSetupBeforeExec(boost::process::executor &e)
{
	//setgid(PluginMonitor::sPluginUid);

	int groupSize = PluginMonitor::sPluginGids.size();
	gid_t groups[groupSize];
	for(int i = 0; i < groupSize; i++)
	{
		groups[i] = PluginMonitor::sPluginGids[i];
		cout << "Added group: " << groups[i] << endl;
	}

	setgroups(groupSize, groups);

	setuid(PluginMonitor::sPluginUid);
}

void PluginMonitor::monitorThreadEntry()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "PluginMonitor", 0, 0, 0);
#endif

	// Delay the starting of any plugins by the time specified in the configuration value.
	{
		uint64_t startTime = TimeUtils::getSystemMillis();

		uint64_t sleepTime;
		do
		{
			usleep(100000);
			char *tailptr;
			sleepTime = strtoull(this->getConfigValue(PLUGINMONITOR_CONFIGKEY_STARTUPDELAY_MS).c_str(), &tailptr, 10);
		} while(TimeUtils::getSystemMillis() < startTime + (sleepTime == 0 ? 10000 : sleepTime));
	}

	this->setPluginStatus(IVP_STATUS_RUNNING);

	while (!boost::this_thread::interruption_requested())
	{
		// Sleep for configurable amount of time TODO: this should be in a function because we use this alot.
		{
			uint64_t startTime = TimeUtils::getSystemMillis();

			uint64_t sleepTime;
			do
			{
				usleep(100000);
				char *tailptr;
				sleepTime = strtoull(this->getConfigValue(PLUGINMONITOR_CONFIGKEY_CHECKINTERVAL_MS).c_str(), &tailptr, 10);
			} while(TimeUtils::getSystemMillis() < startTime + (sleepTime == 0 ? 5000 : sleepTime));
		}

		pthread_mutex_lock(&this->mMessagesLock);

		// Iterate over all messages that have been received since the last time through the main loop of the thread.
		for (map<unsigned int, MessageData>::iterator itrMessage = this->mMessages.begin(); itrMessage != this->mMessages.end(); itrMessage++)
		{
			// Iterate through all plugins that are currently running.
			for (map<InstalledPluginEntry, PluginMonitoringData>::iterator itr = this->mRunningPlugins.begin(); itr != this->mRunningPlugins.end(); itr++)
			{
				// Update the monitoring data for the plugin that corresponds to the message that was received.
				if (itr->first.plugin.id == itrMessage->first)
				{
					// If this is the first message received from the plugin, then add an event log entry.
					// Adding this entry may cause some latency, but this only happens for the first message.
					if (itr->second.lastReceivedTime == 0)
					{
						uint64_t startupTime = itrMessage->second.firstReceivedTime - itr->second.startTime;
						stringstream ss;
						ss << "Plugin '" << itr->first.plugin.name << "' (id: " << itr->first.plugin.id << ") startup time: " << startupTime << " milliseconds";
						this->addEventLogEntry(LogLevel_Info, ss.str());
					}

					// Update the last time a message was received from this plugin.
					itr->second.lastReceivedTime = itrMessage->second.lastReceivedTime;
					break;
				}
			}
		}

		this->mMessages.clear();

		pthread_mutex_unlock(&this->mMessagesLock);

		set<InstalledPluginEntry> enabledPlugins;
		try {
			PluginContext pcontext;
			enabledPlugins = pcontext.getInstalledPlugins(true);

		} catch (const DbException &e) {
			LOG_ERROR("MySQL: Unable to get enabled, installed plugins [" << e.what() << "]");
			continue;
		}

		uint64_t maxStartupTime = 0;
		{
			char *tailptr;
			maxStartupTime = strtoull(this->getConfigValue(PLUGINMONITOR_CONFIGKEY_MAXSTARTUPTIME_MS).c_str(), &tailptr, 10);
		}
		maxStartupTime = maxStartupTime == 0 ? 15000 : maxStartupTime;

		// Remove dead/disabled plugins.
		{
			vector<map<InstalledPluginEntry, PluginMonitoringData>::iterator> deadPlugins;
			for (map<InstalledPluginEntry, PluginMonitoringData>::iterator itr = mRunningPlugins.begin(); itr != mRunningPlugins.end(); itr++)
			{
				waitpid(itr->second.pid, NULL, WNOHANG);
				if (kill(itr->second.pid, 0) != 0)
					deadPlugins.push_back(itr);
			}

			for (unsigned int i = 0; i < deadPlugins.size(); i++)
			{
				map<InstalledPluginEntry, PluginMonitoringData>::iterator itr = deadPlugins[i];
				mRunningPlugins.erase(itr);
			}

			uint64_t currentTime = TimeUtils::getSystemMillis();

			for (map<InstalledPluginEntry, PluginMonitoringData>::iterator itr = mRunningPlugins.begin(); itr != mRunningPlugins.end(); itr++)
			{
				switch(itr->second.killStatus)
				{
				case PluginKillState_sigTermSent:
					kill(itr->second.pid, SIGINT);
					itr->second.killStatus = PluginKillState_sigIntSent;
					break;
				case PluginKillState_sigIntSent:
					kill(itr->second.pid, SIGKILL);
					itr->second.killStatus = PluginKillState_sigKillSent;
					break;
				case PluginKillState_sigKillSent:
					break;
				default:
					bool killIt = false;
					if (enabledPlugins.find(itr->first) == enabledPlugins.end())
					{
						killIt = true;
					}
					else if (itr->second.lastReceivedTime == 0)
					{
						if (itr->second.startTime + maxStartupTime <= currentTime)
						{
							stringstream ss;
							ss << "Plugin '" << itr->first.plugin.name << "' (id: " << itr->first.plugin.id << ") has exceeded the max allowed startup time. Now restarting the plugin.";
							this->addEventLogEntry(LogLevel_Warning, ss.str());

							killIt = true;
						}
					}
					else
					{
						if (itr->first.maxMessageInterval != 0	&& (enabledPlugins.find(itr->first)->maxMessageInterval + itr->second.lastReceivedTime <= currentTime))
						{
							stringstream ss;
							ss << "Plugin '" << itr->first.plugin.name << "' (id: " << itr->first.plugin.id << ") has exceeded it's max message interval. Now restarting the plugin.";
							this->addEventLogEntry(LogLevel_Warning, ss.str());

							killIt = true;
						}
					}
					if (killIt)
					{
						kill(itr->second.pid, SIGTERM);
						itr->second.killStatus = PluginKillState_sigTermSent;
					}

					break;
				}
			}
		}

		for(set<InstalledPluginEntry>::iterator enabledPlugin = enabledPlugins.begin(); enabledPlugin != enabledPlugins.end(); enabledPlugin++)
		{
			if (mRunningPlugins.find(*enabledPlugin) == mRunningPlugins.end())
			{
				vector<string> tokenizedExeName = StringUtils::tokenize(enabledPlugin->exeName, "/", true);
				vector<string> tokenizedPathName = StringUtils::tokenize(enabledPlugin->path, "/", true);
				if (tokenizedExeName.size() == 0)
				{
					try {
						PluginContext pcontext;
						pcontext.disableInstalledPlugin(enabledPlugin->id);
					} catch (const DbException &e) {
						LOG_WARN(e.what());
					}
					this->addEventLogEntry(LogLevel_Fatal, "Unable to start plugin.  Executable name is empty.");
					continue;
				}
				string exeName = tokenizedExeName.at(tokenizedExeName.size() - 1);
				stringstream fullpathStream;
				for(vector<string>::iterator i = tokenizedPathName.begin(); i != tokenizedPathName.end(); i++)
				{
					fullpathStream << "/" << *i;
				}
				string workingDirectory = fullpathStream.str().empty() ? "/" : fullpathStream.str();
				int exeDepth = 0;
				bool outOfRoot = false;
				for(vector<string>::iterator i = tokenizedExeName.begin(); i != tokenizedExeName.end(); i++)
				{
					if (*i != ".")
					{
						fullpathStream << "/" << *i;
						if (*i == "..")
							exeDepth--;
						else
							exeDepth++;
					}
					if (exeDepth < 0)
						outOfRoot = true;

				}
				string fullpath = fullpathStream.str();
				if (exeDepth < 1 || outOfRoot)
				{
					try {
						PluginContext pcontext;
						pcontext.disableInstalledPlugin(enabledPlugin->id);
					} catch (const DbException &e) {
						LOG_ERROR("MySQL: Unable to disable plugin [" << e.what() << "]");
					}
					this->addEventLogEntry(LogLevel_Fatal, "Unable to start plugin. The executable path goes outside of the root of the plugins zip package.");
					continue;
				}

				//TODO: permissions on exe

				try {
					boost::process::child c = boost::process::execute(
								boost::process::initializers::inherit_env(),
								boost::process::initializers::run_exe(fullpath),
								boost::process::initializers::start_in_dir(workingDirectory),
								boost::process::initializers::set_cmd_line(exeName),
								boost::process::initializers::close_fds_if( &PluginMonitor::procSetupCloseAllFd ),
								boost::process::initializers::throw_on_error(),
								boost::process::initializers::on_exec_setup( &PluginMonitor::procSetupBeforeExec ));

					PluginMonitoringData data;
					data.pid = c.pid;
					data.startTime = TimeUtils::getSystemMillis();
					data.lastReceivedTime = 0;
					mRunningPlugins[*enabledPlugin] = data;

				} catch (const std::exception &e) {
					try {
						PluginContext pcontext;
						pcontext.disableInstalledPlugin(enabledPlugin->id);
					} catch (const DbException &e1) {
						LOG_ERROR("MySQL: Unable to disable plugin [" << e.what() << "]");
					}
					this->addEventLogEntry(LogLevel_Fatal, string("Unable to start plugin. Error starting process [") + e.what() + string("]"));
					continue;
				}

				try {
					PluginContext pcontext;
					pcontext.setPluginStatus(enabledPlugin->plugin.id, IVP_STATUS_STARTED);
				} catch (DbException &e) {
					LOG_ERROR("MySQL: Unable to set plugin status [" << e.what() << "]");
				}
			}
		}

		{
			stringstream ss;
			ss << this->mRunningPlugins.size();
			map<string, string> statusItems;
			statusItems[PLUGINMONITOR_STATUSKEY_NUMBERPLUGINS] = ss.str();
			this->setStatusItems(statusItems);
		}
	}
	//todo: cleanup although we probably don't need it.
}
