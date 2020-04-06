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
#include <v2xhubclientAPI/OAIDefaultApi.h>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams



using namespace std;

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace OpenAPI;


namespace CARMACloudPlugin {


class ClientApi: public OAIDefaultApi{

	public:
		QString Qquery; 
		OAIDefaultApi proxy; 
		ClientApi() {

		}
		~ClientApi(){
		}

		void SetProxy(const QString &sh, const QString &host, int port, const QString &basePath){
			proxy.setScheme(sh); 
			proxy.setHost(host); 
			proxy.setPort(port); 
			proxy.setBasePath(basePath);
			//proxy.setTimeOut(10);
		} 

		void SetHeader(char key[100], char val[100])
		{
			proxy.addHeaders(QString::fromLocal8Bit(key),QString::fromLocal8Bit(val));
		}
		
		int SendHeartBeat(char* qr)
		{	
			const QString chk = QString::fromLocal8Bit(qr);
			proxy.v2xhubPost(chk);		
		}
		int SendRequest(string trq)
		{
			//char trq[1000] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest><version>0.1</version><reqseq>99</reqseq><scale>0</scale><bounds><TrafficControlBounds><oldest>1584057600000</oldest><lon>-771521558</lon><lat>389504279</lat><xoffsets>10</xoffsets><xoffsets>20</xoffsets><xoffsets>10</xoffsets><yoffsets>0</yoffsets><yoffsets>500</yoffsets><yoffsets>500</yoffsets></TrafficControlBounds></bounds></TrafficControlRequest>";
			
			proxy.tcmreqPost(QString::fromStdString(trq));	
			
		}

		void successtcmreq()
		{
		}
		void failuretcmreq()
		{

		}



};


class CARMACloudPlugin: public PluginClient {
public:
	CARMACloudPlugin(std::string);
	virtual ~CARMACloudPlugin();
	int Main();
	uint16_t webport;
	std::string webip;
	//ClientApi Cli;  
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	//void OnMessageReceived(IvpMessage *msg);
	void OnStateChange(IvpPluginState state);

	int  StartWebService();
	void CARMAResponseHandler(QHttpEngine::Socket *socket);
	int SendTcmRequest(string s);

	int sendClientcpprest(char msg[], char url[] ,char base[]); 



private:

	pthread_mutex_t _settingsMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t _timMutex = PTHREAD_MUTEX_INITIALIZER;

	uint64_t _frequency = 0;


};
std::mutex _cfgLock;
}
#endif
