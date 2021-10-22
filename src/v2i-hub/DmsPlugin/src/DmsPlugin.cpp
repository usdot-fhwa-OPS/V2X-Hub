/*
 * DmsPlugin.cpp
 *
 *  Created on: Feb 13, 2017
 *      Author: gmb
 */

#include "DmsPlugin.h"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace DMS {

DmsPlugin::DmsPlugin(string name):
		PluginClient(name),
		_isSignMessagesInitialized(false), _isDmsControllerInitialized(false),
		_enableDms(false), _enableSignSimulator(false),
		_msgIdRequested(0), _msgIdShown(-1)
{
	// Add sign message for ID 0.  This is the blanking message.
	AddSignMessage(0, "", false);

	AddMessageFilter(IVPMSG_TYPE_DMSCONT, "MSGID");

	SubscribeToMessages();
}

DmsPlugin::~DmsPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void DmsPlugin::UpdateConfig()
{
	GetConfigValue("Enable Sign Simulator", _enableSignSimulator);
	GetConfigValue("Enable DMS", _enableDms);

	lock_guard<mutex> lock(_cfgLock);

	string ipAddress = _dmsIpAddress;
	uint32_t port = _dmsPort;

	GetConfigValue("DMS IP Address", _dmsIpAddress);
	GetConfigValue("DMS Port", _dmsPort);

	// Set DMS IP address and port number if new values were retrieved.
	if (_dmsIpAddress.compare(ipAddress) != 0 || _dmsPort != port)
	{
		_dmsController.setConfigs(_dmsIpAddress, _dmsPort);
		_isDmsControllerInitialized = true;
		//UninitializeSignMessages();
	}

	ipAddress = _signSimIpAddress;
	port = _signSimPort;

	GetConfigValue("Sign Sim IP Address", _signSimIpAddress);
	GetConfigValue("Sign Sim Port", _signSimPort);

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
			PLOG(logERROR) << "UDP Client for sign simulator could not be created: " << ex.what();
			HandleException(ex, false);
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

		GetConfigValue(key.str().c_str(), message);
		if (!AddSignMessage(i, message, sendToSign))
			sendToSign = false;
	}

}

void DmsPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfig();
	}
}

void DmsPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);

	// Force Message ID is a special configuration value that is used as an action to
	// simulate a message ID being received by the plugin to display that message ID on the sign.
	// So if this key is received process it right away.
	const char* keyForceMessageID = "Force Message ID";
	if (strcmp(key, keyForceMessageID) == 0)
	{
		int32_t forceMessageId = -1;
		GetConfigValue(keyForceMessageID, forceMessageId);
		if (forceMessageId != -1)
		{
			SetRequestedMessage(forceMessageId);
			//ivp_setConfigurationValue(_plugin, keyForceMessageID, "-1");
		}
		else
		{
			SetRequestedMessage(0);
		}
		return;
	}

	UpdateConfig();
}

void DmsPlugin::OnMessageReceived(IvpMessage *msg)
{
	assert(msg != NULL);
	int msgId = -1;

	if ((strcmp(msg->type, IVPMSG_TYPE_DMSCONT) ==0) && (strcmp(msg->subtype, "MSGID")==0) &&(msg->payload->type == cJSON_String))
	{
		msgId = ivpDmsCont_getIvpDmsMsgId(msg);
		SetRequestedMessage(msgId);
	}

	routeable_message rMsg(msg);
	PLOG(logDEBUG) << "Received message: " << rMsg;
}


// Add a sign message to the internal map of available messages.
// Also send the message text to the sign if it is a new message or the message text has changed.
// Return true if the message was sent to the sign or it did not need sent to the sign.
bool DmsPlugin::AddSignMessage(int messageId, string text, bool sendToSign)
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

/// Assign text to the specified message number on the sign.
/// A multi-message is sent to the sign and the text is stored in changeable memory
/// on the sign at the message number location specified.
/// The message text is only displayed on the sign if the activateMessage parameter is true.
/// The text can also be quickly displayed at any time using ActivateMessage().
bool DmsPlugin::AssignMessageOnSign(int messageNumber, const char* message, bool activateMessage)
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
		PLOG(logWARNING) << "AssignMessageOnSign Error, message number cannot be 0.";
		return false;
	}

	// Create message ID (e.g. ".3.2" - 3 = interchangeable memory, 2 = message number);
	// For some reason, strdup must be used below or NTCIP call will fail.
	ostringstream ss;
	ss << ".3." << messageNumber;
	char* msgId = strdup(ss.str().c_str());
	//cout << LOG_PREFIX << "ID: '" << msgId << "'" << endl;

	int statusResponse = 0;

	PLOG(logINFO) << "Sending message. Number: " << messageNumber << ", Text: " << message;

	// Set status to notUsedReq, notifying the sign that interchangeable memory is not used now.
	_dmsController.setDMSMsgStatus("8", msgId);

	statusResponse = _dmsController.getDMSMsgStatus(msgId);

	// A status of 1 means it was set to unused.
	if (statusResponse != 1)
	{
		PLOG(logERROR) << "Set status to 8 - Invalid status retrieved: " << statusResponse;
		free(msgId);
		msgId = NULL; // This will prevent freeing the same memory again
		return false;
	}
	
	free(msgId);

	// Set status to modifyReq, notifying the sign to use modifiable memory location for message number 2.
	_dmsController.setDMSMsgStatus("6", msgId);

	statusResponse = _dmsController.getDMSMsgStatus(msgId);

	// A response of 2 means it is in modifiable mode.
	if (statusResponse != 2)
	{
		PLOG(logERROR) << "Set status to 6 - Invalid status response retrieved: " << statusResponse;
		free(msgId);
		msgId = NULL; // This will prevent freeing the same memory again
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
		PLOG(logERROR) << "Set status to 7 - Invalid status response retrieved: " << statusResponse;
		free(msgId);
		msgId = NULL; // This will prevent freeing the same memory again
		return false;
	}

	if (activateMessage)
		return ActivateMessage(messageNumber);

	return true;
}

// Activate the message number specified.
// If messageNumber is 0, the sign is blanked instead.
bool DmsPlugin::ActivateMessage(int messageNumber)
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
		PLOG(logINFO) << "Blanking Message";
	else
		PLOG(logINFO) << "Activating Message: " << messageNumber;

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

	PLOG(logDEBUG) << "Message Activation Code: " << buffer;

	if (_dmsController.setDMSMsgActivate(buffer))
	{
		_msgIdShown = messageNumber;
		return true;
	}

	return false;
}

bool DmsPlugin::ShowSignMessage(int msgId)
{
	if (!_isSignMessagesInitialized)
		return false;

	PLOG(logINFO) << "Setting sign to message ID: " << msgId;

	std::map<int, SignMessage>::iterator messageIter = _signMessages.find(msgId);
	if (messageIter == _signMessages.end())
	{
		PLOG(logERROR) << "Invalid sign message ID: " << msgId;
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

bool DmsPlugin::SetRequestedMessage(int msgId)
{
	// Lock this method from executing more than once at a time.
	unique_lock<mutex> lock(_mutexSetRequested);

	_msgIdRequested = msgId;

	PLOG(logDEBUG) << "Current message id shown: " << _msgIdShown << ". Requested message id: " << _msgIdRequested;

	if (_msgIdRequested == _msgIdShown)
		return true;

	bool success = ShowSignMessage(msgId);

	if (success)
		_msgIdShown = msgId;

	return success;
}

// Return true if the DMS is enabled and all messages are properly initialized.
bool DmsPlugin::InitializeSignMessages()
{
	// If the DMS is not enabled, there is nothing more to do.
	// But return false so that the initialization will happen if the DMS is enabled.
	if (!_enableDms)
		return false;

	// Initialize the sign with each message if the init has not been performed yet for that message.

	lock_guard<mutex> lock(_cfgLock);

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

void DmsPlugin::UninitializeSignMessages()
{
	lock_guard<mutex> lock(_cfgLock);

	std::map<int, SignMessage>::iterator it;

	for (it = _signMessages.begin(); it != _signMessages.end(); it++)
	{
		it->second.IsDmsTextInitialized = false;
	}

	_isSignMessagesInitialized = false;
}


void DmsPlugin::UpdateMsgIdStatus(const char* key, int messageId)
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

	SetStatus<string>(key, ss.str());
}

int DmsPlugin::Main()
{
	PLOG(logINFO) << "Starting Plugin";

	while (_plugin->state != IvpPluginState_error)
	{
		if (_isSignMessagesInitialized)
		{
			if (_msgIdRequested != _msgIdShown)
				SetRequestedMessage(_msgIdRequested);
		}
		else
		{
			_isSignMessagesInitialized = InitializeSignMessages();
		}

		SetStatus("Is Sign Initialized", (_isSignMessagesInitialized == true));
		UpdateMsgIdStatus("Message Requested", _msgIdRequested);
		UpdateMsgIdStatus("Message Shown", _msgIdShown);

		sleep(1);
	}

	return EXIT_SUCCESS;
}

} /* namespace DMS */

int main(int argc, char **argv) {
	return run_plugin<DMS::DmsPlugin>("DynamicMessageSign", argc, argv);
}
