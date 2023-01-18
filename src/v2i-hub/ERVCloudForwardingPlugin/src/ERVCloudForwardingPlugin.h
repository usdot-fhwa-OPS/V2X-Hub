#ifndef ERVCLOUDFORWARDINGPLUGIN_H_
#define ERVCLOUDFORWARDINGPLUGIN_H_

#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpJ2735.h>
#include "PluginUtil.h"
#include "PluginClient.h"
#include <curl/curl.h>
#include <qhttpengine/server.h>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHostAddress>
#include <QRegExp>
#include <QStringList>
#include <QSharedPointer>
#include <QObject>
#include <qhttpengine/server.h>
#include <v2xhubWebAPI/OAIApiRouter.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "ERVCloudForwardingWorker.h"
#include "SNMPClient.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace ERVCloudForwardingPlugin
{
    class ERVCloudForwardingPlugin : public PluginClient
    {
    private:
        uint16_t _webPort;
        string _webIp;
        string _rsuIp;
        string _rsuName;
        uint16_t _snmpPort;
        uint32_t _rsuInterval;
        string _securityUser;
        string _authPassPhrase;
        string _GPSOID;
        const string _CLOUDURL = "http://127.0.0.1:33333";
        const string _CLOUDBSMREQ = "/carmacloud/rsu/req";
        const string _CLOUDRSUREQ = "/carmacloud/rsu/register";
        const string _POSTMETHOD = "POST";
        const string _HEXENC = "asn.1-uper/hexstring";
        const uint16_t SECTOMILLISEC = 1000;

    public:
        explicit ERVCloudForwardingPlugin(const string &);
        ~ERVCloudForwardingPlugin() = default;
        int Main();

    protected:
        void UpdateConfigSettings();
        void OnConfigChanged(const char *key, const char *value);
        void OnStateChange(IvpPluginState state);
        int StartBSMWebService();
        void CARMACloudResponseHandler(QHttpEngine::Socket *socket);
        /**
         * @brief Message Filter handler for BSM
         * @param msg BSM message
         * @param routableMsg
         */
        void handleBSM(BsmMessage &msg, routeable_message &routableMsg);
        /**
         * @brief Send BSM request to the cloud
         * @param msg BSM message request
         * @param url The server IP and port the request is sent to
         * @param base The request path
         * @param method HTTP method POST
         * @return int http status indicator
         */
        int CloudSend(const string &msg, const string &url, const string &base, const string &method);
        /**
         * @brief Create a new thread and send BSM request to the cloud 
         * @param msg BSM message request
         * @param url The server IP and port the request is sent to
         * @param base The request path
         * @param method HTTP method POST
         */
        void CloudSendAsync(const string &msg, const string &url, const string &base, const string &method);
        void BroadcastBSM(const string &bsmHex);
        /**
         * @brief Send SNMP request to RSU to get RSU GPS location, then send the RSU GPS location to the carma-cloud
         */
        void RegisterRSULocation();
    };
} // namespace ERVCloudForwardingPlugin
std::mutex _cfgLock;

#endif