//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================
#include <string.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"

#include <atomic>
#include <thread>
#include <DecodedBsmMessage.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <PersonalSafetyMessage.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/j2735_messages/PersonalSafetyMessage.hpp>

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


using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;
using namespace OpenAPI;

namespace PedestrianPlugin
{

/**
 * This plugin is an example to demonstrate the capabilities of a TMX plugin.
 */
class PedestrianPlugin: public PluginClient
{
public:
	PedestrianPlugin(std::string);
	PedestrianPlugin();
	virtual ~PedestrianPlugin();
	int Main();
	uint16_t webport;
	std::string webip; 
	std::string webSocketIP;
	std::string webSocketURLExt;
	std::string dataprovider;
	float cameraRotation;
	std::shared_ptr<FLIRWebSockAsyncClnSession> flirSession;

protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

	void HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);
	void BroadcastPsm(char *psmJson);

	int  StartWebService();
	void PedestrianRequestHandler(QHttpEngine::Socket *socket);
	void writeResponse(int responseCode , QHttpEngine::Socket *socket);

	int StartWebSocket();

	void OnWebSocketConnected();
	void OnWebSocketDataReceived(QString message);
	void OnWebSocketClosed();
	
	int checkXML();



private:
	tmx::utils::UdpClient *_signSimClient = NULL;
	J2735MessageFactory factory;
	

};
std::mutex _cfgLock;

};

