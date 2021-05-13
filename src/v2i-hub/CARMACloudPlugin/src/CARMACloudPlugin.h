/*
 * TimPlugin.h
 *
 *  Created on: October 25, 2017
 *      Author: zinkg
 */

#ifndef CARMACLOUDPLUGIN_H_
#define CARMACLOUDPLUGIN_H_

#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <atomic>
#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpJ2735.h>
#include <GeoVector.h>
#include <boost/date_time.hpp>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>


#include "PluginUtil.h"
#include "PluginClient.h"

#include <ApplicationMessage.h>
#include <ApplicationDataMessage.h>

#include <tmx/messages/auto_message.hpp>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/testMessage04.hpp>
#include <TestMessage04.h>
#include <tmx/j2735_messages/testMessage05.hpp>
#include <TestMessage05.h>
#include <tmx/messages/auto_message.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>


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
#include <v2xhubWebAPI/OAIApiRouter.h>


#include <curl/curl.h>




using namespace std;

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace OpenAPI;


namespace CARMACloudPlugin {


class CARMACloudPlugin: public PluginClient {
public:
	CARMACloudPlugin(std::string);
	virtual ~CARMACloudPlugin();
	int Main();
	uint16_t webport;
	std::string webip;
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	//void OnMessageReceived(IvpMessage *msg);
	void OnStateChange(IvpPluginState state);

	int  StartWebService();
	void CARMAResponseHandler(QHttpEngine::Socket *socket);
	int CloudSend(string msg,string url, string base, string method);
	string updateTags(string s,string t, string t1);

	void HandleCARMARequest(tsm4Message &msg, routeable_message &routeableMsg);
	void GetInt32(unsigned char *buf, int32_t *value)
	{
		*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
	}
	
	void GetInt64(unsigned char *buf, uint64_t *value)
	{
		*value = (uint64_t)((buf[0] << 56) + (buf[1] << 48) + (buf[2] << 40) + (buf[3] << 32) + (buf[4] << 24) + (buf[5] << 16) + (buf[6] << 8) + buf[7]);
	}



private:

	pthread_mutex_t _settingsMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t _timMutex = PTHREAD_MUTEX_INITIALIZER;
	J2735MessageFactory factory;
	uint64_t _frequency = 0;
	string url,base_hb, base_req, method; 


};
std::mutex _cfgLock;
}
#endif
