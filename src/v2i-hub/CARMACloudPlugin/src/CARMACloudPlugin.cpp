#include "CARMACloudPlugin.h"
#include <WGS84Point.h>

using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;
using namespace boost::property_tree;

namespace CARMACloudPlugin {



/**
 * Construct a new CARMACloudPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
CARMACloudPlugin::CARMACloudPlugin(string name) :PluginClient(name) {

		// xml parser setup 
		//this->Cli.SetProxy(QString::fromLocal8Bit("http"),QString::fromLocal8Bit("127.0.0.1"),22222,QString::fromLocal8Bit("/v1/carmacloud"),0);
        std::lock_guard<mutex> lock(_cfgLock);
		GetConfigValue<string>("WebServiceIP",webip);
		GetConfigValue<uint16_t>("WebServicePort",webport);

		std::thread webthread(&CARMACloudPlugin::StartWebService,this);
		webthread.detach(); // wait for the thread to finish 

}

CARMACloudPlugin::~CARMACloudPlugin() {
}

void CARMACloudPlugin::CARMAResponseHandler(QHttpEngine::Socket *socket)
{
	cout<<"CARMA:: Server acquired\n";
	QString st; 
	while(socket->bytesAvailable()>0)
	{	
		st.append(socket->readAll());
	}
	QByteArray array = st.toLocal8Bit();

	char* _cloudUpdate = array.data(); // would be the cloud update packet, needs parsing
 
	cout <<"CARMA:: Response :: "<<_cloudUpdate<<endl;
}



int CARMACloudPlugin::StartWebService()
{
	//Web services 
	char *placeholderX[1]={0};
	int placeholderC=1;
	QCoreApplication a(placeholderC,placeholderX);

 	QHostAddress address = QHostAddress(QString::fromStdString (webip));
    quint16 port = static_cast<quint16>(webport);

	QSharedPointer<OpenAPI::OAIApiRequestHandler> handler(new OpenAPI::OAIApiRequestHandler());
	handler = QSharedPointer<OpenAPI::OAIApiRequestHandler> (new OpenAPI::OAIApiRequestHandler());

	auto router = QSharedPointer<OpenAPI::OAIApiRouter>::create();
    router->setUpRoutes();

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {

		CARMAResponseHandler(socket);
    });

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {

		router->processRequest(socket);
    });

    QHttpEngine::Server server(handler.data());

    if (!server.listen(address, port)) {
        qCritical("Unable to listen on the specified port.");
        return 1;
    }
	PLOG(logDEBUG4)<<"CARMACloudPlugin:: Started web service";
	return a.exec();

}


void CARMACloudPlugin::UpdateConfigSettings() {

	GetConfigValue<uint64_t>("Frequency", _frequency);

	
    std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue<string>("WebServiceIP",webip);
	GetConfigValue<uint16_t>("WebServicePort",webport);

	
}

void CARMACloudPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void CARMACloudPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}


int CARMACloudPlugin::CloudSend(string msg,string url, string base, string method)
{
	CURL *req;
  	CURLcode res;
	string urlfull = url+base;	  


  	req = curl_easy_init();
 	 if(req) {
  	  	curl_easy_setopt(req, CURLOPT_URL, urlfull.c_str());

		if(strcmp(method.c_str(),"POST")==0)
		{
    		curl_easy_setopt(req, CURLOPT_POSTFIELDS, msg.c_str());
			res = curl_easy_perform(req);
   			if(res != CURLE_OK)
			   {
      				fprintf(stderr, "curl send failed: %s\n",curl_easy_strerror(res));
					  return 1;
			   }	  
		}
    	curl_easy_cleanup(req);
  }
  return 0;
}



int CARMACloudPlugin::Main() {


	FILE_LOG(logINFO) << "Starting plugin.";
	
	std::string msg = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest><version>0.1</version><reqseq>99</reqseq><scale>0</scale><bounds><TrafficControlBounds><oldest>1584057600000</oldest><lon>-771521558</lon><lat>389504279</lat><xoffsets>10</xoffsets><xoffsets>20</xoffsets><xoffsets>10</xoffsets><yoffsets>0</yoffsets><yoffsets>500</yoffsets><yoffsets>500</yoffsets></TrafficControlBounds></bounds></TrafficControlRequest>";
	std::string url ="http://127.0.0.1:22222";
	std::string base = "/carmacloud/v2xhub";
	std::string method = "POST";
	

	while (_plugin->state != IvpPluginState_error) {

		if (IsPluginState(IvpPluginState_registered))
		{
			CloudSend(msg,url, base, method);
			this_thread::sleep_for(chrono::milliseconds(2000));
		}

	}

	return (EXIT_SUCCESS);
}
} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < CARMACloudPlugin::CARMACloudPlugin > ("CARMACloudPlugin", argc, argv);
}
