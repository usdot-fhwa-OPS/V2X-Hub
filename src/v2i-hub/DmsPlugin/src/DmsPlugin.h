/*
 * DmsPlugin.h
 *
 *  Created on: Feb 13, 2017
 *      Author: gmb
 */

#ifndef DMSPLUGIN_H_
#define DMSPLUGIN_H_

#include <atomic>
#include <mutex>
#include <PluginClient.h>
#include <tmx/messages/IvpDmsControlMsg.h>
#include <UdpClient.h>

#include "dmsNTCIP.h"

namespace DMS {

struct SignMessage
{
	// Text suitable for display in the sign simulator application.
	// '\n' used to separate multiple lines.
	std::string SimulatorText;

	// Text sent of NTCIP to a Dynamic Message Sign.
	// Text is formatted using codes in brackets '[]'.
	std::string DmsText;

	// True if the DmsText has been successfully initialized on the sign.
	bool IsDmsTextInitialized;
};

class DmsPlugin: public tmx::utils::PluginClient {
public:
	DmsPlugin(std::string name);
	virtual ~DmsPlugin();

	int Main();
protected:
	void UpdateConfig();
	void UpdateMsgIdStatus(const char *, int);

	// DMS Controller and messages API
	bool AddSignMessage(int, std::string, bool);
	bool AssignMessageOnSign(int, const char*, bool);
	bool ActivateMessage(int);
	bool ShowSignMessage(int);
	bool SetRequestedMessage(int);
	bool InitializeSignMessages();
	void UninitializeSignMessages();

	// Overrides
	void OnConfigChanged(const char *key, const char *value);
	void OnMessageReceived(IvpMessage *msg);
	void OnStateChange(IvpPluginState state);
private:
	std::mutex _cfgLock;
	std::mutex _mutexSetRequested;

	SignalControllerNTCIP _dmsController;
	tmx::utils::UdpClient *_signSimClient = NULL;

	std::atomic<bool> _isSignMessagesInitialized;
	std::atomic<bool> _isDmsControllerInitialized;
	std::atomic<bool> _enableDms;
	std::atomic<bool> _enableSignSimulator;

	std::string _dmsIpAddress = "";
	uint32_t _dmsPort = 0;
	std::string _signSimIpAddress = "";
	uint32_t _signSimPort = 0;

	std::map<int, SignMessage> _signMessages;

	// The last message ID that was requested to be shown on the sign.
	std::atomic<int> _msgIdRequested;
	// The last message ID that was actually shown on the sign.
	std::atomic<int> _msgIdShown;
};

} /* namespace DMS */

#endif /* DMSPLUGIN_H_ */
