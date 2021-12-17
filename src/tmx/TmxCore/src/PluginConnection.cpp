/*
 * PluginConnection.cpp
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#ifndef __CYGWIN__
#include <sys/prctl.h>
#endif
#include "PluginConnection.h"
#include "tmx/tmx.h"
#include "tmx/IvpPlugin.h"
#include "tmx/utils/MsgFramer.h"
#include "utils/PerformanceTimer.h"
#include <assert.h>
using namespace std;

// The PluginConnection class is instantiated by ivpcore when a Plugin opens a socket to ivpcore
// using the ivpapi library.
// The receiver thread then listens for messages over the socket.
// When a message is received it is placed on a queue for processing by the processor threads.

PluginConnection::PluginConnection(MessageRouter *router, int socket) : Plugin(router)
{
	assert(socket != (int) NULL);

	this->mSocket = socket;

	mReceiverThread = boost::thread(&PluginConnection::receiverThread, this);
	mFastProcessorThread = boost::thread(&PluginConnection::fastProcessorThread, this);
	mSlowProcessorThread = boost::thread(&PluginConnection::slowProcessorThread, this);
}

PluginConnection::~PluginConnection()
{

}

void PluginConnection::onConfigChanged(string key, string value)
{
	IvpConfigCollection *collection = nullptr;
	collection = ivpConfig_addItemToCollection(collection, key.c_str(), value.c_str(), nullptr);

	IvpMessage *msg = ivpConfig_createMsg(collection);
	if (msg)
	{
		this->onMessageReceived(msg);
		ivpMsg_destroy(msg);
	}
}

// This onMessageReceived override is called to send messages to plugins that are using the ivpapi
// and are connected to ivpcore via network socket.
// Internal plugins like MessageProfiler and PluginMonitor directly derive from Plugin and override
// onMessageReceived.
void PluginConnection::onMessageReceived(IvpMessage *msg)
{
	struct pollfd poll_data;

	char *jsonmsg = ivpMsg_createJsonString(msg, IvpMsg_FormatOptions_none);
	if (jsonmsg)
	{
		int framedMsgLength;
		char *framedMsg = msgFramer_createFramedMsg(jsonmsg, strlen(jsonmsg), &framedMsgLength);

		if (framedMsg)
		{
			if (mSocket != (int) NULL)
			{
				poll_data.fd = mSocket;
				poll_data.events = POLLOUT;

				poll(&poll_data,1,1);

				if(!(poll_data.revents & POLLHUP))
				{
					int retvalue = write(mSocket,framedMsg,framedMsgLength);
					if (retvalue < 0)
						shutdown(mSocket, SHUT_RDWR);
				}
			}

			free(framedMsg);
		}

		free(jsonmsg);
	}
}

// The receiver thread reads messages sent from a plugin to ivpcore over a socket.
// Each message received is placed on a queue that is handled by one of the two processor threads.
void PluginConnection::receiverThread()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "PluginConReceiver", 0, 0, 0);
#endif

	MsgFramer framer = MSG_FRAMER_INITIALIZER;

	boost::this_thread::disable_interruption di;

	while (!boost::this_thread::interruption_requested())
	{
		usleep(100);

		int recvcount = recv(mSocket, msgFramer_getBuf(&framer), msgFramer_getBufLength(&framer), 0);
		if (recvcount <= 0)	//connection has been closed
		{
			cout << "GUI connection closing..." << recvcount << endl;
			close(mSocket);

			mFastProcessorThread.interrupt();
			mSlowProcessorThread.interrupt();
			mEventContinueFastProcessor.Set();
			mEventContinueSlowProcessor.Set();
			mFastProcessorThread.join();
			mSlowProcessorThread.join();

			delete this;
			return;
		}

		msgFramer_incrementBufPos(&framer, recvcount);

		char *rawMessage = nullptr;

		while ((rawMessage = msgFramer_getNextMsg(&framer)) != nullptr)
		{
			// Create an IvpMessage from the raw message.
			IvpMessage *msg = ivpMsg_parse(rawMessage);

			// If the message could not be parsed, send an error message back to the plugin.
			if (msg == nullptr)
			{
				IvpMessage *errMsg = ivpError_createMsg(ivpError_createError(IvpLogLevel_warn, IvpError_messageParse, 0));
				if (errMsg)
				{
					this->onMessageReceived(errMsg);
					ivpMsg_destroy(errMsg);
				}
				continue;
			}

			// Place the message on the appropriate queue for processing by another thread.
			// Non-critical messages that are slower to process are placed on the slow processor thread
			// and the others are placed on the fast processor thread.
			// For instance, in one case, writing of status messages to the database was taking 9 ms.
			// That is why status messages and event messages are processed in their own thread.
			// Note that the IvpMessage is freed in the processor threads.

			if (ivpPluginStatus_isStatusMsg(msg) ||	ivpEventLog_isEventLogMsg(msg))
			{
				mMutexSlowMessageQueue.lock();
				mSlowMessageQueue.push(msg);
				mMutexSlowMessageQueue.unlock();
				mEventContinueSlowProcessor.Set();
			}
			else
			{
				mMutexFastMessageQueue.lock();
				mFastMessageQueue.push(msg);
				mMutexFastMessageQueue.unlock();
				mEventContinueFastProcessor.Set();
			}
		}
	}
}

// The fast processor thread handles all critical IVP messages.
// Any non-critical messages that are slow to process are instead handled by the slow processor thread.
void PluginConnection::fastProcessorThread()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "PluginConFastProcessor", 0, 0, 0);
#endif

	bool messageWaiting = false;
	IvpMessage *msg = nullptr;

	// Disable interruption of this thread (as long as the variable below is in scope).
	// This allows the thread to exit gracefully by checking interruption_requested().
	boost::this_thread::disable_interruption di;

	while (!boost::this_thread::interruption_requested())
	{
		// If a message is not already waiting, sleep so this thread does not consume the CPU.
		if (!messageWaiting)
			mEventContinueFastProcessor.WaitOne();

		// Lock the queue and retrieve the message.

		mMutexFastMessageQueue.lock();

		if (!mFastMessageQueue.empty())
		{
			msg = mFastMessageQueue.front();
			mFastMessageQueue.pop();
			messageWaiting = !mFastMessageQueue.empty();
		}
		else
		{
			msg = nullptr;
		}

		mMutexFastMessageQueue.unlock();

		if (msg == nullptr) continue;

		// These system messages (registration, subscribe, and config) are handled on this fast processor thread.
		// This ensures no important message get out of order.

		if (ivpRegister_isRegistrationMsg(msg))
		{
			processRegistrationMessage(msg);
		}
		else if (ivpSubscribe_isSubscribeMsg(msg))
		{
			processSubscribeMessage(msg);
		}
		else if (ivpConfig_isConfigMsg(msg))
		{
			processConfigMessage(msg);
		}
		else
		{
			// This is not a system message, but a normal message.
			// Route it to all subscribers (this includes internal subscribers like the plugin monitor and history manager).
			try
			{
				this->sendMessageToRouter(msg);
			}
			catch (PluginException &e)
			{
				IvpError error = IVP_ERROR_INITIALIZER;
				error.level = IvpLogLevel_error;
				//TODO: Error number
				IvpMessage *errMsg = ivpError_createMsg(error);
				if (errMsg)
				{
					this->onMessageReceived(errMsg);
					ivpMsg_destroy(errMsg);
				}
			}
		}

		ivpMsg_destroy(msg);
	}
}

// The slow processor thread handles all non-critical IVP messages.
// Any critical messages are instead handled by the fast processor thread.
// It processes messages that can take some time, but where it does not matter
// if they are executed out-of-order from other messages received.
void PluginConnection::slowProcessorThread()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "PluginConSlowProcessor", 0, 0, 0);
#endif

	bool messageWaiting = false;
	IvpMessage *msg = nullptr;

	// Disable interruption of this thread (as long as the variable below is in scope).
	// This allows the thread to exit gracefully by checking interruption_requested().
	boost::this_thread::disable_interruption di;

	while (!boost::this_thread::interruption_requested())
	{
		// If a message is not already waiting, sleep so this thread does not consume the CPU.
		if (!messageWaiting)
			mEventContinueSlowProcessor.WaitOne();

		// Lock the queue and retrieve the message.

		mMutexSlowMessageQueue.lock();

		if (!mSlowMessageQueue.empty())
		{
			msg = mSlowMessageQueue.front();
			mSlowMessageQueue.pop();
			messageWaiting = !mSlowMessageQueue.empty();
		}
		else
		{
			msg = nullptr;
		}

		mMutexSlowMessageQueue.unlock();

		if (msg == nullptr) continue;

		if (ivpPluginStatus_isStatusMsg(msg))
		{
			processStatusMessage(msg);
		}
		else if (ivpEventLog_isEventLogMsg(msg))
		{
			processEventLogMessage(msg);
		}

		ivpMsg_destroy(msg);
	}
}

void PluginConnection::processRegistrationMessage(IvpMessage *msg)
{
	IvpManifest *manifest = ivpRegister_getManifest(msg);

	if (manifest)
	{
		IvpError manifestError = ivpRegister_validateManifest(manifest);
		if (manifestError.error == IvpError_none)
		{
			RegistrationInformation info;

			if (manifest->name) info.pluginInfo.name = string(manifest->name);
			if (manifest->description) info.pluginInfo.description = string(manifest->description);
			if (manifest->version) info.pluginInfo.version = string(manifest->version);

			{
				int arraySize = ivpConfig_getItemCount(manifest->configuration);
				for (int i = 0; i < arraySize; i++)
				{
					IvpConfigItem *item = ivpConfig_getItem(manifest->configuration, i);
					assert(item != NULL);
					string description = item->description ? string(item->description) : "";
					info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(string(item->key), string(item->defaultValue), description));

					ivpConfig_destroyConfigItem(item);
				}
			}
			{
				int arraySize = ivpMsgType_getEntryCount(manifest->messageTypes);
				for (int i = 0; i < arraySize; i++)
				{
					IvpMessageTypeEntry *entry = ivpMsgType_getEntry(manifest->messageTypes, i);
					assert(entry != NULL);

					MessageTypeEntry dbEntry(string(entry->type), string(entry->subtype));
					dbEntry.description = entry->description ? string(entry->description) : "";
					info.messageTypeEntries.push_back(dbEntry);

					ivpMsgType_destroyEntry(entry);
				}
			}

			try
			{
				this->registerPlugin(info);

				map<string, PluginConfigurationParameterEntry> configEntries = this->getAllConfigValues();
				IvpConfigCollection *collection = nullptr;
				for (map<string, PluginConfigurationParameterEntry>::iterator itr = configEntries.begin(); itr != configEntries.end(); itr++)
				{
					if (itr->second.value != itr->second.defaultValue)
						collection = ivpConfig_addItemToCollection(collection, itr->second.key.c_str(), itr->second.value.c_str(), NULL);
				}
				IvpMessage *msg = ivpConfig_createMsg(collection);
				if (msg)
				{
					this->onMessageReceived(msg);
					ivpMsg_destroy(msg);
				}
			}
			catch (PluginException &e)
			{
				IvpError error = IVP_ERROR_INITIALIZER;
				error.level = IvpLogLevel_fatal;
				IvpMessage *errMsg = ivpError_createMsg(error);
				if (errMsg)
				{
					this->onMessageReceived(errMsg);
					ivpMsg_destroy(errMsg);
				}
			}
		}
		else
		{
			LOG_FATAL("Error validating manifest");
		}

		ivpRegister_destroyManifest(manifest);
	}
	else
	{
		LOG_FATAL("Error Extracting manifest");
	}
}

void PluginConnection::processSubscribeMessage(IvpMessage *msg)
{
	vector<MessageFilterEntry> filterEntries;

	int entryCount = ivpSubscribe_getFilterEntryCount(msg);

	for (int i = 0; i < entryCount; i++)
	{
		char *type, *subtype;
		IvpMsgFlags flagmask;
		ivpSubscribe_getFilterEntry(msg, i, &type, &subtype, &flagmask);

		if (type != nullptr)
		{
			MessageFilterEntry filterEntry;
			filterEntry.type = string(type);
			if (subtype != nullptr)
			{
				filterEntry.subtype = string(subtype);
			}
			filterEntry.flagmask = flagmask;
			filterEntries.push_back(filterEntry);
		}
	}

	try {
		this->subscribeForMessages(filterEntries);
	}
	catch (PluginException &e)
	{
		IvpError error = IVP_ERROR_INITIALIZER;
		error.level = IvpLogLevel_error;
		IvpMessage *errMsg = ivpError_createMsg(error);

		if (errMsg)
		{
			this->onMessageReceived(errMsg);
			ivpMsg_destroy(errMsg);
		}
	}
}

void PluginConnection::processConfigMessage(IvpMessage *msg)
{
	IvpConfigCollection *collection = msg->payload;
	int arraySize = ivpConfig_getItemCount(collection);

	ConfigContext ccontext;
	vector<PluginConfigurationParameterEntry> globalParams;

	for (int i = 0; i < arraySize; i++)
	{
		IvpConfigItem *item = ivpConfig_getItem(collection, i);
		assert(item != NULL);
		assert(item->key != NULL);
		assert(item->value != NULL);

		// If this is a global system parameter, then update the appropriate table
		if (strcmp(msg->source, "_global") == 0)
		{
			PluginConfigurationParameterEntry entry(item->key, item->defaultValue,
					item->description ? item->description : "Global system parameter");
			entry.value = item->value;
			globalParams.push_back(entry);
		}
		else
		{
			this->setConfigValue(item->key, item->value);
		}

		ivpConfig_destroyConfigItem(item);
	}

	try
	{
		ccontext.initializePluginConfigParameters(0, globalParams);
	}
	catch (DbException &e)
	{
		LOG_FATAL("<Plugin System> MySQL: Unable to add global plugin configuration parameter to database [" << e.what() << "]");
		throw PluginException("Unable to add global plugin configuration parameter [" + string(e.what()) + "]");
	}
}

void PluginConnection::processStatusMessage(IvpMessage *msg)
{
	int arraySize = ivpPluginStatus_getItemCount(msg->payload);
	vector<string> removeItems;
	map<string, string> updateItems;

	for (int i = 0; i < arraySize; i++)
	{
		IvpPluginStatusItem *item = ivpPluginStatus_getItem(msg->payload, i);
		assert(item != NULL);

		if (item->value == nullptr)
		{
			assert(item->key);
			if (item->key && strlen(item->key) > 0)
				removeItems.push_back(string(item->key));
		}
		else
		{
			string key;
			if (item->key)
				key = string(item->key);
			updateItems[key] = string(item->value);
		}
	}

	try
	{
		this->setStatusItems(updateItems);
		this->removeStatusItems(removeItems);
	}
	catch(PluginException &e)
	{
		LOG_WARN(e.what());
	}
}

void PluginConnection::processEventLogMessage(IvpMessage *msg)
{
	IvpEventLogEntry *eventLogEntry = ivpEventLog_getEventLogEntry(msg);
	assert(eventLogEntry != NULL);

	if (eventLogEntry)
	{
		assert(eventLogEntry->description != NULL);
		assert(eventLogEntry->description[0] != '\0');

		if (eventLogEntry != nullptr && eventLogEntry->description[0] != '\0')
		{
			this->addEventLogEntry((LogLevel)eventLogEntry->level, string(eventLogEntry->description));
		}

		ivpEventLog_destoryEventLogEntry(eventLogEntry);
	}
}
