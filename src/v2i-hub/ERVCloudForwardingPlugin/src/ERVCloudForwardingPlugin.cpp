#include "ERVCloudForwardingPlugin.h"

namespace ERVCloudForwardingPlugin
{
    ERVCloudForwardingPlugin::ERVCloudForwardingPlugin(const string &name) : PluginClient(name)
    {
        UpdateConfigSettings();
        std::lock_guard<mutex> lock(_cfgLock);
        AddMessageFilter<BsmMessage>(this, &ERVCloudForwardingPlugin::handleBSM);
        // Subscribe to all messages specified by the filters above.
        SubscribeToMessages();

        std::thread webBSM_t(&ERVCloudForwardingPlugin::StartBSMWebService, this);
        webBSM_t.detach();
        // Send RSU location to cloud at one hour interval 
        std::thread webRegisterRSU_t(&ERVCloudForwardingPlugin::PeriodicRSURegisterReq, this);
        webRegisterRSU_t.detach();
    }

    void ERVCloudForwardingPlugin::handleBSM(BsmMessage &msg, routeable_message &routableMsg)
    {
        // Check if the BSM is broadcast by an ERV (Emergency Response Vehicle)
        if (ERVCloudForwardingWorker::IsBSMFromERV(msg))
        {
            // Construct the ERV BSM and forward it to the cloud.
            auto xml_str = ERVCloudForwardingWorker::constructERVBSMRequest(msg);
            PLOG(logINFO) << "Forward ERV BSM to cloud: " << xml_str << endl;
            CloudSendAsync(xml_str, _CLOUDURL, _CLOUDBSMREQ, _POSTMETHOD);
        }
        else
        {
            // If BSM is not from ERV, print debug log
            PLOG(logDEBUG) << "Incoming BSM is not from Emergency Response Vehicle (ERV)." << endl;
        }
    }

    void ERVCloudForwardingPlugin::RegisterRSULocation()
    {
        uint32_t attempt = 0;
        bool isRegistered = false;
        while (attempt < _max_rsu_connection_attempt && !isRegistered)
        {
            attempt++;
            this_thread::sleep_for(chrono::seconds(1));
            PLOG(logINFO) << "Attempting to register RSU " << attempt << " times." << endl;
            try
            {
                PLOG(logINFO) << "Create SNMP Client to connect to RSU. RSU IP:" << _rsuIp << ",\tRSU Port:" << _snmpPort << ",\tSecurity Name:" << _securityUser << ",\tAuthentication Passphrase: " << _authPassPhrase << endl;
                auto snmpClient = std::make_shared<SNMPClient>(_rsuIp, _snmpPort, _securityUser, _authPassPhrase);
                auto gps_sentence = snmpClient->SNMPGet(_GPSOID);
                auto gps_map = ERVCloudForwardingWorker::ParseGPS(gps_sentence);
                long latitude = 0;
                long longitude = 0;
                for (auto itr = gps_map.begin(); itr != gps_map.end(); itr++)
                {
                    latitude = itr->first;
                    longitude = itr->second;
                }

                if (latitude == 0 || longitude == 0)
                {
                    PLOG(logERROR) << "Invalid latitude and longitude. Cannot register RSU location." << endl;
                    continue;
                }
                auto uuid = boost::uuids::random_generator()();
                string rsu_identifier = _rsuName + "_" + boost::lexical_cast<std::string>(uuid);
                auto xml_str = ERVCloudForwardingWorker::constructRSULocationRequest(rsu_identifier, _webPort, latitude, longitude);
                PLOG(logINFO) << "Sending registering RSU location reqest to cloud: " << xml_str << endl;
                auto status = CloudSend(xml_str, _CLOUDURL, _CLOUDRSUREQ, _POSTMETHOD);
                if (status == 1)
                {
                    PLOG(logERROR) << "Cannot register RSU location. Reason: Failed to send RSU location to cloud." << endl;
                    continue;
                }
                isRegistered = true;
            }
            catch (SNMPClientException &ex)
            {
                PLOG(logERROR) << "Cannot register RSU location. Reason: " << ex.what() << endl;
            }
        }
        if (isRegistered)
        {
            PLOG(logINFO) << "Successfully registered RSU location!" << endl;
        }else{
            PLOG(logERROR) << "Failed to register RSU after trying " << attempt << " times." << endl;
        }
    }

    void ERVCloudForwardingPlugin::PeriodicRSURegisterReq()
    {
        while (true)
        {   
            RegisterRSULocation();
            this_thread::sleep_for(chrono::seconds(_rsuInterval));  
        }
    }

    void ERVCloudForwardingPlugin::UpdateConfigSettings()
    {
        std::lock_guard<mutex> lock(_cfgLock);
        GetConfigValue<string>("WebServiceIP", _webIp);
        GetConfigValue<uint16_t>("WebServicePort", _webPort);
        GetConfigValue<string>("RSUIp", _rsuIp);
        GetConfigValue<uint16_t>("SNMPPort", _snmpPort);
        GetConfigValue<string>("SecurityUser", _securityUser);
        GetConfigValue<string>("AuthPassPhrase", _authPassPhrase);
        GetConfigValue<string>("GPSOID", _GPSOID);
        GetConfigValue<string>("RSUName", _rsuName);
    }

    void ERVCloudForwardingPlugin::BroadcastBSM(const string &bsmHex)
    {
        std::unique_ptr<BsmEncodedMessage> msg;
        J2735MessageFactory factory;
        msg.reset();
        msg.reset(dynamic_cast<BsmEncodedMessage *>(factory.NewMessage(api::MSGSUBTYPE_BASICSAFETYMESSAGE_STRING)));
        msg->set_payload(bsmHex);
        msg->set_flags(IvpMsgFlags_RouteDSRC);
        msg->addDsrcMetadata(0x20);
        msg->set_encoding(_HEXENC);
        msg->refresh_timestamp();
        auto rMsg = dynamic_cast<routeable_message *>(msg.get());
        BroadcastMessage(*rMsg);
        PLOG(logDEBUG) << "Broadcast ERV BSM:" << msg->get_payload() << endl;
    }

    void ERVCloudForwardingPlugin::CARMACloudResponseHandler(QHttpEngine::Socket *socket)
    {
        QString st;
        while (socket->bytesAvailable() > 0)
        {
            auto readBytes = socket->readAll();
            st.append(readBytes);
        }
        QByteArray array = st.toLocal8Bit();
        const char *_cloudUpdate = array.data();
        string bsmHex = _cloudUpdate;
        PLOG(logINFO) << "Received ERV BSM from cloud:" << bsmHex << endl;
        BroadcastBSM(bsmHex);
    }

    int ERVCloudForwardingPlugin::StartBSMWebService()
    {
        PLOG(logINFO) << "ERVCloudForwardingPlugin:: Starting web service...";
        //In case some configuration for web service changes, the web service wait for those change before starting
        this_thread::sleep_for(chrono::seconds(_max_web_service_waiting));
        char *placeholderX[1] = {nullptr};
        int placeholderC = 1;
        QCoreApplication a(placeholderC, placeholderX);
        auto address = QHostAddress(QString::fromStdString(_webIp));
        auto port = static_cast<quint16>(_webPort);
        QSharedPointer<OpenAPI::OAIApiRequestHandler> handler(new OpenAPI::OAIApiRequestHandler());
        handler = QSharedPointer<OpenAPI::OAIApiRequestHandler>(new OpenAPI::OAIApiRequestHandler());
        auto router = QSharedPointer<OpenAPI::OAIApiRouter>::create();
        router->setUpRoutes();
        QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket)
                         { CARMACloudResponseHandler(socket); });
        QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket)
                         { router->processRequest(socket); });
        QHttpEngine::Server server(handler.data());
        if (!server.listen(address, port))
        {
            qCritical("Unable to listen on the specified port.");
            return EXIT_FAILURE;
        }
        PLOG(logINFO) << "ERVCloudForwardingPlugin:: Started web service!";
        return a.exec();
    }

    void ERVCloudForwardingPlugin::CloudSendAsync(const string &local_msg, const string &local_url, const string &local_base, const string &local_method)
    {
        std::thread t([this, local_msg, &local_url, &local_base, &local_method]()
                      { CloudSend(local_msg, local_url, local_base, local_method); });
        t.detach();
    }

    int ERVCloudForwardingPlugin::CloudSend(const string &local_msg, const string &local_url, const string &local_base, const string &local_method)
    {
        CURL *req;
        CURLcode res;
        string urlfull = local_url + local_base;
        req = curl_easy_init();
        if (req)
        {
            curl_easy_setopt(req, CURLOPT_URL, urlfull.c_str());
            if (strcmp(local_method.c_str(), "POST") == 0)
            {
                curl_easy_setopt(req, CURLOPT_POSTFIELDS, local_msg.c_str());
                curl_easy_setopt(req, CURLOPT_TIMEOUT_MS, 1000L);
                curl_easy_setopt(req, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
                res = curl_easy_perform(req);
                if (res != CURLE_OK)
                {
                    fprintf(stderr, "curl send failed: %s\n", curl_easy_strerror(res));
                    return EXIT_FAILURE;
                }
            }
            curl_easy_cleanup(req);
        }
        return 0;
    }

    void ERVCloudForwardingPlugin::OnConfigChanged(const char *key, const char *value)
    {
        PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }

    void ERVCloudForwardingPlugin::OnStateChange(IvpPluginState state)
    {
        PluginClient::OnStateChange(state);
        if (state == IvpPluginState_registered)
        {
            UpdateConfigSettings();
        }
    }

    int ERVCloudForwardingPlugin::Main()
    {
        PLOG(logINFO) << "Starting ERVCloudForwardingPlugin.";
        while (_plugin->state != IvpPluginState_error)
        {

            if (IsPluginState(IvpPluginState_registered))
            {
                this_thread::sleep_for(chrono::milliseconds(5000));
            }
        }
        return EXIT_SUCCESS;
    }
}

int main(int argc, char *argv[])
{
    return run_plugin<ERVCloudForwardingPlugin::ERVCloudForwardingPlugin>("ERVCloudForwardingPlugin", argc, argv);
}
