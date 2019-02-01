//============================================================================
// Name        : DMSPlugin.cpp
// Author      : Battelle Memorial Institute
// Version     :
// Copyright   : Copyright (c) 2014 Battelle Memorial Institute. All rights reserved.
// Description : Plugin to send messages to a Dynamic Message Sign.
//============================================================================

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include "tmx/tmx.h"
#include "tmx/IvpPlugin.h"
#include <tmx/messages/IvpNmea.h>
#include <tmx/messages/IvpDmsControlMsg.h>
#include "dmsNTCIP.h"
#include "UdpClient.h"
#include "PluginUtil.h"
#include "ManualResetEvent.h"

using namespace std;
using namespace tmx::utils;

const char* LOG_PREFIX = "DMSPlugin: ";

ManualResetEvent _eventNewConfig;

mutex _mutexSetRequested;

IvpPlugin *_plugin = NULL;

SignalControllerNTCIP _dmsController;
UdpClient *_signSimClient = NULL;

bool _enableSignSimulator = true;
bool _enableDms = true;
string _dmsIpAddress = "";
uint32_t _dmsPort = 0;
string _signSimIpAddress = "";
uint32_t _signSimPort = 0;

struct SignMessage
{
	// Text suitable for display in the sign simulator application.
	// '\n' used to separate multiple lines.
	string SimulatorText;

	// Text sent of NTCIP to a Dynamic Message Sign.
	// Text is formatted using codes in brackets '[]'.
	string DmsText;

	// True if the DmsText has been successfully initialized on the sign.
	bool IsDmsTextInitialized;
};

map<int, SignMessage> _signMessages;

bool _isDmsControllerInitialized = false;
atomic_bool _isSignMessagesInitialized(false);

// The last message ID that was requested to be shown on the sign.
int _msgIdRequested = 0;
// The last message ID that was actually shown on the sign.
int _msgIdShown = -1;

// Forward declarations.
bool AssignMessageOnSign(int messageNumber, const char* message, bool activateMessage);
bool SetRequestedMessage(int msgId);
bool ActivateMessage(int messageNumber);
void UninitializeSignMessages();

// Add a sign message to the internal map of available messages.
// Also send the message text to the sign if it is a new message or the message text has changed.
// Return true if the message was sent to the sign or it did not need sent to the sign.
bool AddSignMessage(int messageId, string text, bool sendToSign)
{
	// Sign messages are formatted in the configuration parameters.
	// They contain NTCIP 1203 formatting like below.
	//
	// [jl3] - Justify Line centered.
	// [pt15o0] - Page Timing. t15 - on time 1.5 seconds (15 tenths). o0 - off time 0 seconds (tenths).
	// [np] - New Page.
	//
	// For example: "[jl3][pt15o0]25[np]MPH".

	// Replace the newline formatting with a newline for use with the simulator.
	// TODO: other formatting should be stripped for sign sim text.
	string simText = text;
	boost::replace_all(simText, "[np]", "\n");

	std::map<int, SignMessage>::iterator it = _signMessages.find(messageId);

	if (it == _signMessages.end())
	{
		SignMessage message;
		message.DmsText = text;
		message.SimulatorText = simText;
		message.IsDmsTextInitialized = false;

		_isSignMessagesInitialized = false;

		_signMessages.insert(std::pair<int, SignMessage>(messageId, message));
		it = _signMessages.find(messageId);
	}
	else
	{
		if (it->second.DmsText != text)
		{
			it->second.DmsText = text;
			it->second.SimulatorText = simText;
			it->second.IsDmsTextInitialized = false;

			_isSignMessagesInitialized = false;
		}
	}

	// If the DMS is not enabled, or the text did not change, there is nothing more to do.
	if (!_enableDms || !_isDmsControllerInitialized || it->second.IsDmsTextInitialized || !sendToSign)
		return true;

	// 0 is just the blanking message.  Nothing needs initialized.
	if (messageId == 0)
	{
		it->second.IsDmsTextInitialized = true;
		return true;
	}

	if (!it->second.IsDmsTextInitialized)
	{
		// Assign the message text to changeable memory on the sign without displaying it.
		if (AssignMessageOnSign(it->first, it->second.DmsText.c_str(), false))
			it->second.IsDmsTextInitialized = true;
		else
			return false;
	}

	return true;
}

// Get configuration settings.
// This method is only called from the main thread.
void GetConfigSettings(IvpPlugin *plugin)
{
	PluginUtil::GetConfigValue(plugin, "Enable DMS", &_enableDms);
	PluginUtil::GetConfigValue(plugin, "Enable Sign Simulator", &_enableSignSimulator);

	string ipAddress = _dmsIpAddress;
	uint32_t port = _dmsPort;

	PluginUtil::GetConfigValue(plugin, "DMS IP Address", &_dmsIpAddress);
	PluginUtil::GetConfigValue(plugin, "DMS Port", &_dmsPort);

	// Set DMS IP address and port number if new values were retrieved.
	if (_dmsIpAddress.compare(ipAddress) != 0 || _dmsPort != port)
	{
		_dmsController.setConfigs(_dmsIpAddress, _dmsPort);
		_isDmsControllerInitialized = true;
		UninitializeSignMessages();
	}

	ipAddress = _signSimIpAddress;
	port = _signSimPort;

	PluginUtil::GetConfigValue(plugin, "Sign Sim IP Address", &_signSimIpAddress);
	PluginUtil::GetConfigValue(plugin, "Sign Sim Port", &_signSimPort);

	// Create the UDP client used to send messages to the sign simulator if new values were retrieved.
	if (_signSimIpAddress.compare(ipAddress) != 0 || _signSimPort != port)
	{
		try
		{
			if (_signSimClient != NULL)
				delete _signSimClient;

			_signSimClient = new UdpClient(_signSimIpAddress, _signSimPort);
		}
		catch (UdpClientRuntimeError &ex)
		{
			cout << LOG_PREFIX << "UDP Client for sign simulator could not be created." << endl;
			cout << LOG_PREFIX << "Exception: " << ex.what() << endl;
		}
	}

	// Retrieve the sign messages and add them to the available messages.
	// Send them to the sign so that they are ready for activation.
	// If the send fails for any of them, stop sending them since the config is wrong
	// or there are other issues.  The main loop will try again later in this case.

	string message;
	bool sendToSign = true;

	for (int i = 1; i <= 4; i++)
	{
		ostringstream key;
		key << "Message " << setfill('0') << setw(2) << i;

		PluginUtil::GetConfigValue(plugin, key.str().c_str(), &message);
		if (!AddSignMessage(i, message, sendToSign))
			sendToSign = false;
	}
}

/// Assign text to the specified message number on the sign.
/// A multi-message is sent to the sign and the text is stored in changeable memory
/// on the sign at the message number location specified.
/// The message text is only displayed on the sign if the activateMessage parameter is true.
/// The text can also be quickly displayed at any time using ActivateMessage().
bool AssignMessageOnSign(int messageNumber, const char* message, bool activateMessage)
{
	// If the last message requested is the message on the sign that is changing,
	// then reset the flag that says it has been shown.
	if (_msgIdRequested == messageNumber)
		_msgIdShown = -1;

	// For help, see document "NTCIP 1203 version v03".
	// Page 127 (page number 112) talks about activating a message.
	// Page 176 (page number 161) talks about message status.

	// For message status:
	// 1 = notUsed, 2 = modifying, 3 = validating, 4 = valid,
	// 5 = error, 6 = modifyReq, 7 = validateReq, 8 = notUsedReq.

	if (messageNumber == 0)
	{
		cout << LOG_PREFIX << "AssignMessageOnSign Error, message number cannot be 0." << endl;
		return false;
	}

	// Create message ID (e.g. ".3.2" - 3 = interchangeable memory, 2 = message number);
	// For some reason, strdup must be used below or NTCIP call will fail.
	ostringstream ss;
	ss << ".3." << messageNumber;
	char* msgId = strdup(ss.str().c_str());
	//cout << LOG_PREFIX << "ID: '" << msgId << "'" << endl;

	int statusResponse = 0;

	cout << LOG_PREFIX << "Sending message. Number: " << messageNumber << ", Text: " << message << endl;

	// Set status to notUsedReq, notifying the sign that interchangeable memory is not used now.
	_dmsController.setDMSMsgStatus("8", msgId);

	statusResponse = _dmsController.getDMSMsgStatus(msgId);

	// A status of 1 means it was set to unused.
	if (statusResponse != 1)
	{
		cout << LOG_PREFIX << "Set status to 8 - Invalid status retrieved: " << statusResponse << endl;
		free(msgId);
		return false;
	}

	// Set status to modifyReq, notifying the sign to use modifiable memory location for message number 2.
	_dmsController.setDMSMsgStatus("6", msgId);

	statusResponse = _dmsController.getDMSMsgStatus(msgId);

	// A response of 2 means it is in modifiable mode.
	if (statusResponse != 2)
	{
		cout << LOG_PREFIX << "Set status to 6 - Invalid status response retrieved: " << statusResponse << endl;
		free(msgId);
		return false;
	}

	// Set the string for interchangeable memory at the message number specified by the ID.
	_dmsController.setDMSMsgMultiString(message, msgId);

	_dmsController.setDMSMsgOwner("TTI", msgId);

	// Set to highest priority.
	_dmsController.setDMSMsgRunTimePriority("1", msgId);

	// Validate the message and calculate the checksum.

	// Set statusResponse to validateReq.
	_dmsController.setDMSMsgStatus("7", msgId);

	statusResponse = _dmsController.getDMSMsgStatus(msgId);

	if (statusResponse != 4)
	{
		cout << LOG_PREFIX << "Set status to 7 - Invalid status response retrieved: " << statusResponse << endl;
		free(msgId);
		return false;
	}

	free(msgId);

	if (activateMessage)
		return ActivateMessage(messageNumber);

	return true;
}

// Activate the message number specified.
// If messageNumber is 0, the sign is blanked instead.
bool ActivateMessage(int messageNumber)
{
	int memoryType = 3;
	bool blankMessage = false;

	if (messageNumber == 0)
	{
		messageNumber = 1;
		blankMessage = true;
		memoryType = 7;
	}

	char buffer[80];

	if (blankMessage)
		cout << LOG_PREFIX << "Blanking Message" << endl;
	else
		cout << LOG_PREFIX << "Activating Message: " << messageNumber << endl;

	// Get the checksum for message number specified.
	sprintf(buffer, ".%d.%d", memoryType, messageNumber);
	int crc = 0;
	if (!blankMessage)
		crc = _dmsController.getDMSMsgCRCCurr(buffer);

	// Activation code.
	// 2 bytes: Duration in minutes. ffff is highest value.
	// 1 byte: priority. ff is highest priority.
	// 1 byte: type of memory. 03 is changeable memory.
	// 2 bytes: message number, 0002. Sign could be queried for how many messages it actually supports.
	// 2 bytes: checksum
	// 4 bytes: IP address of the sender, although seems to accept anything.

	sprintf(buffer, "ffffff%02d%04d%04xc0a0010a", memoryType, messageNumber, crc);

	//cout << LOG_PREFIX << "Message Activation Code: " << buffer << endl;

	if (_dmsController.setDMSMsgActivate(buffer))
	{
		_msgIdShown = messageNumber;
		return true;
	}

	return false;
}

bool ShowSignMessage(int msgId)
{
	if (!_isSignMessagesInitialized)
		return false;

	cout << LOG_PREFIX << "Setting sign to message ID: " << msgId << endl;

	std::map<int, SignMessage>::iterator messageIter = _signMessages.find(msgId);
	if (messageIter == _signMessages.end())
	{
		cout << LOG_PREFIX << "Invalid sign message ID: " << msgId << endl;
		return false;
	}

	SignMessage signMessage = messageIter->second;

	bool success = true;

	if (_enableDms)
	{
		// Activate the message on the DMS using NTCIP.
		if (!ActivateMessage(msgId))
			success = false;
	}

	if (_enableSignSimulator)
	{
		if (_signSimClient != NULL)
		{
			char buffer[240];
			sprintf(buffer, "{\"DisplayText\":\"%s\"}", signMessage.SimulatorText.c_str());

			_signSimClient->Send(buffer, strlen(buffer));
		}
	}

	return success;
}

bool SetRequestedMessage(int msgId)
{
	// Lock this method from executing more than once at a time.
	unique_lock<mutex> lock(_mutexSetRequested);

	_msgIdRequested = msgId;

	if (_msgIdRequested == _msgIdShown)
		return true;

	bool success = ShowSignMessage(_msgIdRequested);

	if (success)
		_msgIdShown = _msgIdRequested;

	return success;
}

void onStateChange(IvpPlugin *plugin, IvpPluginState state)
{
	cout << LOG_PREFIX << "State Change: " << PluginUtil::IvpPluginStateToString(state) << endl;

	if (state == IvpPluginState_registered)
	{
		_eventNewConfig.Set();
	}
}

void onError(IvpPlugin *plugin, IvpError err)
{
	cerr << LOG_PREFIX << err.level << " - " << err.error << ", " << err.sysErrNo << endl;
}

void onMessageReceived(IvpPlugin *plugin, IvpMessage *msg)
{
	assert(msg != NULL);

	if ((strcmp(msg->type, IVPMSG_TYPE_DMSCONT) ==0) && (strcmp(msg->subtype, "MSGID")==0) &&(msg->payload->type == cJSON_String))
	{
		int msgId = ivpDmsCont_getIvpDmsMsgId(msg);

		//cout << LOG_PREFIX << "MsgId: " << msgId << ", Last MsgId: " << lastMsgId << endl;

		SetRequestedMessage(msgId);
	}
}

void onConfigChanged(IvpPlugin *plugin, const char *key, const char *value)
{
	cout << LOG_PREFIX << "Config Changed - " << key << ": " << value << endl;

	// Force Message ID is a special configuration value that is used as an action to
	// simulate a message ID being received by the plugin to display that message ID on the sign.
	// So if this key is received process it right away.
	const char* keyForceMessageID = "Force Message ID";
	if (strcmp(key, keyForceMessageID) == 0)
	{
		int32_t forceMessageId = -1;
		PluginUtil::GetConfigValue(plugin, keyForceMessageID, &forceMessageId);
		if (forceMessageId != -1)
		{
			SetRequestedMessage(forceMessageId);
			ivp_setConfigurationValue(plugin, keyForceMessageID, "-1");
		}
		return;
	}

	// All other configuration values are processed by the main thread.
	// Set the event to wake it up.
	_eventNewConfig.Set();
}

// Return true if the DMS is enabled and all messages are properly initialized.
bool InitializeSignMessages()
{
	// If the DMS is not enabled, there is nothing more to do.
	// But return false so that the initialization will happen if the DMS is enabled.
	if (!_enableDms)
		return false;

	// Initialize the sign with each message if the init has not been performed yet for that message.

	bool success = true;
	std::map<int, SignMessage>::iterator it;

	for (it = _signMessages.begin(); it != _signMessages.end(); it++)
	{
		if (it->first == 0) continue;

		if (!it->second.IsDmsTextInitialized)
		{
			// Assign the message text to changeable memory on the sign without displaying it.
			if (AssignMessageOnSign(it->first, it->second.DmsText.c_str(), false))
				it->second.IsDmsTextInitialized = true;
			else
			{
				success = false;
				// If one fails, then there is likely a network error or a wrong configuration.
				// Abort.
				break;
			}
		}
	}

	return success;
}

void UninitializeSignMessages()
{
	std::map<int, SignMessage>::iterator it;

	for (it = _signMessages.begin(); it != _signMessages.end(); it++)
	{
		it->second.IsDmsTextInitialized = false;
	}

	_isSignMessagesInitialized = false;
}

void UpdateMsgIdStatus(const char* key, int messageId)
{
	std::ostringstream ss;
	ss << "ID " << messageId << " = ";

	std::map<int, SignMessage>::iterator it = _signMessages.find(messageId);

	if (messageId == 0)
		ss << "blank";
	else if (it == _signMessages.end())
		ss << "unknown message";
	else
		ss << it->second.DmsText;

	PluginUtil::SetStatus<string>(_plugin, key, ss.str());
}

int main()
{
	cout << LOG_PREFIX << "Starting DMS Plugin" << endl;

	// Add sign message for ID 0.  This is the blanking message.
	AddSignMessage(0, "", false);

	IvpPluginInformation info = IVP_PLUGIN_INFORMATION_INITIALIZER;
	info.onMsgReceived = onMessageReceived;
	info.onStateChange = onStateChange;
	info.onError = onError;
	info.onConfigChanged = onConfigChanged;

	_plugin = ivp_create(info);

	if (!_plugin)
	{
		cerr << LOG_PREFIX << "Error creating DMS Plugin" << endl;
		return EXIT_FAILURE;
	}

	IvpMsgFilter *filter = ivpSubscribe_addFilterEntry(NULL, IVPMSG_TYPE_DMSCONT, "MSGID");
	ivp_subscribe(_plugin, filter);
	ivpSubscribe_destroyFilter(filter);

	// Sleep until configuration data is available for the first time.
	// This corresponds to when the plugin state changes to registered in onStateChange().
	cout << LOG_PREFIX << "Waiting for Registered state." << endl;
	_eventNewConfig.WaitOne();
	_eventNewConfig.Reset();

	// Get the initial configuration settings, including the IP addresses and ports.
	GetConfigSettings(_plugin);

	while (_plugin->state != IvpPluginState_error)
	{
		// Sleep until new configuration data is available or timeout.
		bool isNewConfig = _eventNewConfig.WaitOne(chrono::milliseconds(500));

		if (isNewConfig)
		{
			_eventNewConfig.Reset();

			GetConfigSettings(_plugin);
		}

		if (_isSignMessagesInitialized)
		{
			if (_msgIdRequested != _msgIdShown)
				SetRequestedMessage(_msgIdRequested);
		}
		else
		{
			_isSignMessagesInitialized = InitializeSignMessages();
		}

		PluginUtil::SetStatus<bool>(_plugin, "Is Sign Initialized", _isSignMessagesInitialized);
		UpdateMsgIdStatus("Message Requested", _msgIdRequested);
		UpdateMsgIdStatus("Message Shown", _msgIdShown);
	}

	if (_signSimClient != NULL)
		delete _signSimClient;

	return EXIT_SUCCESS;
}
