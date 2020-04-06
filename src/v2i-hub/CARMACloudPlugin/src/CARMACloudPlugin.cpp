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

int CARMACloudPlugin::SendTcmRequest(string test)
{
	char *placeholderX[1]={0};
	int placeholderC=1;
	QCoreApplication b(placeholderC,placeholderX);
	ClientApi c; 
	char key[100]="Content-Type";
	char val[100]="text/plain";
	std::string m = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest><version>0.1</version><reqseq>99</reqseq><scale>0</scale><bounds><TrafficControlBounds><oldest>1584057600000</oldest><lon>-771521558</lon><lat>389504279</lat><xoffsets>10</xoffsets><xoffsets>20</xoffsets><xoffsets>10</xoffsets><yoffsets>0</yoffsets><yoffsets>500</yoffsets><yoffsets>500</yoffsets></TrafficControlBounds></bounds></TrafficControlRequest>";

	c.SetProxy(QString::fromLocal8Bit("http"),QString::fromLocal8Bit("127.0.0.1"),22222,QString::fromLocal8Bit("/carmacloud"));
	c.SetHeader(key,val);
	c.SendRequest(m);
	return b.exec();
	 
}

int CARMACloudPlugin::sendClientcpprest(char msg[],char url[],char base[])
{
		// http_client client(U(url));

		// web::json::value json_return;

        // // Build request URI and start the request.
        // uri_builder builder(U(base));
        // //builder.append_query(U(""), U("cpprestsdk github"));
        // return client.request(methods::POST, builder.to_string(),msg,U("text/plain"))
		// .then([=](http_response response)
    	// {
        // 	printf("Received response status code:%u\n", response.status_code());
		// 	return response.status_code();

        // 	// Write response body into the file.
        // 	//return response.body().read_to_end(fileStream->streambuf());
    	// })
		// .then([](pplx::task<json::value> previousTask)
      	// {
        //  	try
        //  	{
        //     	//display_json(previousTask.get(), L"R: ");
        //  	}
        //  	catch (http_exception const & e)
        // 	 {
        //     	wcout << e.what() << endl;
        // 	 }
      	// })
      	// .wait();


}


int CARMACloudPlugin::Main() {


	FILE_LOG(logINFO) << "Starting plugin.";
	ClientApi c; 
	char key[100]="Content-Type";
	char val[100]="text/plain";
	c.SetProxy(QString::fromLocal8Bit("http"),QString::fromLocal8Bit("127.0.0.1"),11111,QString::fromLocal8Bit("/carmacloud"));
	//c.SetHeader(key,val);
	std::string test = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest><version>0.1</version><reqseq>99</reqseq><scale>0</scale><bounds><TrafficControlBounds><oldest>1584057600000</oldest><lon>-771521558</lon><lat>389504279</lat><xoffsets>10</xoffsets><xoffsets>20</xoffsets><xoffsets>10</xoffsets><yoffsets>0</yoffsets><yoffsets>500</yoffsets><yoffsets>500</yoffsets></TrafficControlBounds></bounds></TrafficControlRequest>";
	//CARMACloudPlugin cc; 
	char *placeholderX[1]={0};
	int placeholderC=1;

	

	while (_plugin->state != IvpPluginState_error) {

		if (IsPluginState(IvpPluginState_registered))
		{
			//QCoreApplication b(placeholderC,placeholderX);
			//SendTcmRequest(test);
			//std::thread sendthread(&CARMACloudPlugin::SendTcmRequest,this,test);
			//sendthread.detach(); // wait for the thread to finish 
			//b.exec();
			//this_thread::sleep_for(chrono::milliseconds(5000));
			usleep(2000000);
		}

	}

	return (EXIT_SUCCESS);
}
} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < CARMACloudPlugin::CARMACloudPlugin > ("CARMACloudPlugin", argc, argv);
}
