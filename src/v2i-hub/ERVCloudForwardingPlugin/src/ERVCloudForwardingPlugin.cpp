#include "ERVCloudForwardingPlugin.h"
#include "ERVCloudForwardingWorker.h"

namespace ERVCloudForwardingPlugin
{
    ERVCloudForwardingPlugin::ERVCloudForwardingPlugin(string name) : PluginClient(name)
    {
        UpdateConfigSettings();
        std::lock_guard<mutex> lock(_cfgLock);
        AddMessageFilter<BsmMessage>(this, &ERVCloudForwardingPlugin::handleBSM);
        // Subscribe to all messages specified by the filters above.
        SubscribeToMessages();

        std::thread webBSM_t(&ERVCloudForwardingPlugin::StartBSMWebService, this);
        webBSM_t.detach();
    }

    void ERVCloudForwardingPlugin::handleBSM(BsmMessage &msg, routeable_message &routableMsg)
    {
        // Check if the BSM is broadcast by an ERV (Emergency Response Vehicle)
        if (ERVCloudForwardingWorker::IsBSMFromERV(msg))
        {
            // Construct the ERV BSM and forward it to the cloud.
            auto xml_str = ERVCloudForwardingWorker::constructERVBSMRequest(msg);
            PLOG(logINFO) << "Forward ERV BSM to cloud: " << xml_str << endl;
            CloudSendAsync(xml_str, _CLOUDURL, _CLOUDBSMREQ, _METHOD);
        }
        else
        {
            // If BSM is not from ERV, print debug log
            PLOG(logDEBUG) << "Incoming BSM is not from Emergency Response Vehicle (ERV)." << endl;
        }
    }

    void ERVCloudForwardingPlugin::UpdateConfigSettings()
    {
        std::lock_guard<mutex> lock(_cfgLock);
        GetConfigValue<string>("WebServiceIP", _webIp);
        GetConfigValue<uint16_t>("WebServicePort", _webPort);
    }

    void ERVCloudForwardingPlugin::BroadcastBSM(const string &bsmHex)
    {
        std::unique_ptr<BsmEncodedMessage> msg;
        J2735MessageFactory factory;
        msg.reset();
        msg.reset(dynamic_cast<BsmEncodedMessage *>(factory.NewMessage(api::MSGSUBTYPE_BASICSAFETYMESSAGE_STRING)));
        msg->set_payload(bsmHex);
        msg->set_flags(IvpMsgFlags_RouteDSRC);
        msg->addDsrcMetadata(0x8002);
        msg->set_encoding(_HEXENC);
        msg->refresh_timestamp();
        routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
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
        char *_cloudUpdate = array.data();
        string bsmHex = _cloudUpdate;
        PLOG(logINFO) << "Received ERV BSM from cloud:" << bsmHex << endl;
        BroadcastBSM(bsmHex);
    }

    int ERVCloudForwardingPlugin::StartBSMWebService()
    {
        char *placeholderX[1] = {0};
        int placeholderC = 1;
        QCoreApplication a(placeholderC, placeholderX);
        QHostAddress address = QHostAddress(QString::fromStdString(_webIp));
        quint16 port = static_cast<quint16>(_webPort);
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
            return 1;
        }
        PLOG(logINFO) << "ERVCloudForwardingPlugin:: Started web service";
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
                res = curl_easy_perform(req);
                if (res != CURLE_OK)
                {
                    fprintf(stderr, "curl send failed: %s\n", curl_easy_strerror(res));
                    return 1;
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
        FILE_LOG(logINFO) << "Starting ERVCloudForwardingPlugin.";
        while (_plugin->state != IvpPluginState_error)
        {

            if (IsPluginState(IvpPluginState_registered))
            {
                this_thread::sleep_for(chrono::milliseconds(5000));
            }
        }
        return (EXIT_SUCCESS);
    }

    ERVCloudForwardingPlugin::~ERVCloudForwardingPlugin()
    {
    }
}

int main(int argc, char *argv[])
{
    return run_plugin<ERVCloudForwardingPlugin::ERVCloudForwardingPlugin>("ERVCloudForwardingPlugin", argc, argv);
}
