//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================
#pragma once
#include <string.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <PersonalSafetyMessage.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

#include <UdpClient.h>
#include <tmx/messages/auto_message.hpp>
#include "PedestrianPluginWorker.hpp"

#include "FLIRWebSockAsyncClnSession.hpp"


#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHostAddress>
#include <QRegExp>
#include <QStringList>
#include <QSharedPointer>
#include <QObject>

#ifdef __linux__
#include <signal.h>
#include <unistd.h>
#endif
#include <qhttpengine/server.h>
#include <qserverPedestrian/OAIApiRouter.h>
#include <qserverPedestrian/OAIPSM.h>
#include <queue>

namespace PedestrianPlugin
{

/**
 * This plugin is an example to demonstrate the capabilities of a TMX plugin.
 */
class PedestrianPlugin: public PluginClient
{
public:
	explicit PedestrianPlugin(const std::string &name);
	int Main() override;

protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value) override;
	void OnStateChange(IvpPluginState state) override;

	void BroadcastPsm(const std::string &psmJson);

	int  StartWebService();
	int  StartWebSocket();
	void StopWebService();
	void StopWebSocket();
	void PedestrianRequestHandler(QHttpEngine::Socket *socket);
	void writeResponse(int responseCode , QHttpEngine::Socket *socket) const;

	void OnWebSocketConnected();
	void OnWebSocketDataReceived(QString message);
	void OnWebSocketClosed();

	[[noreturn]] int checkXML();
	
private:
	std::unique_ptr<tmx::utils::UdpClient> _signSimClient = nullptr;
	
	J2735MessageFactory factory;
	
	std::mutex _cfgLock;
	
	uint16_t webport;
	std::string webip; 
	std::string webSocketIP;
	std::string webSocketURLExt;
	int instance;
	std::string dataprovider;
	float cameraRotation;
	std::shared_ptr<FLIRWebSockAsyncClnSession> flirSession;
	std::string hostString;

	std::thread webSocketThread;
	std::thread xmlThread;
	std::thread webServiceThread;

	std::atomic<bool> runningWebSocket;
	std::atomic<bool> runningWebService;

	// The io_context is required for all I/O
    net::io_context ioc;

	// API URL to accept PSM XML
	const QString PSM_Receive = "";
};

};

