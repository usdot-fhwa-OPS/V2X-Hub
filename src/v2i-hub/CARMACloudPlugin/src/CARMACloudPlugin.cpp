#include "CARMACloudPlugin.h"
#include <WGS84Point.h>
#include <math.h>
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
		AddMessageFilter < tsm4Message > (this, &CARMACloudPlugin::HandleCARMARequest);

		// Subscribe to all messages specified by the filters above.
		SubscribeToMessages();
		std::thread webthread(&CARMACloudPlugin::StartWebService,this);
		webthread.detach(); // wait for the thread to finish 
		url ="http://127.0.0.1:33333"; // 33333 is the port that will send from v2xhub to carma cloud ## initally was 23665
		base_hb = "/carmacloud/v2xhub";
		base_req = "/carmacloud/tcmreq";
		method = "POST";

}


void CARMACloudPlugin::HandleCARMARequest(tsm4Message &msg, routeable_message &routeableMsg)
{
	auto carmaRequest = msg.get_j2735_data();

	// create an XML template for the request
	//if(carmaRequest->body.present == TrafficControlRequest_PR_tcrV01) // taking this out since some message arent enabling this present variable. 
	//{

		unsigned char *reqid=new unsigned char [carmaRequest->body.choice.tcrV01.reqid.size+1];
		memcpy(reqid,carmaRequest->body.choice.tcrV01.reqid.buf, carmaRequest->body.choice.tcrV01.reqid.size+1);
		long int reqseq = carmaRequest->body.choice.tcrV01.reqseq;
		long int scale = carmaRequest->body.choice.tcrV01.scale;
		

		int totBounds =  carmaRequest->body.choice.tcrV01.bounds.list.count;
		int cnt=0;
		char bounds_str[5000]; 

		while(cnt<totBounds)
		{
			int32_t oldest=0;
			GetInt32((unsigned char*)carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->oldest.buf,&oldest);
			// = (int*)  carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->oldest.buf;
			long lat = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->reflat; 
			long longg = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->reflon;

		
			long dtx0 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[0]->deltax;
			long dty0 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[0]->deltay;
			long dtx1 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[1]->deltax;
			long dty1 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[1]->deltay;
			long dtx2 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[2]->deltax;
			long dty2 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[2]->deltay;

			sprintf(bounds_str+strlen(bounds_str),"<bounds><oldest>%d</oldest><reflon>%ld</reflon><reflat>%ld</reflat><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets></bounds>",oldest,longg,lat,dtx0,dty0,dtx1,dty1,dtx2,dty2);

			cnt++;


		}

		char xml_str[10000]; 

	sprintf(xml_str,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest><reqid>%ld</reqid><reqseq>%ld</reqseq><scale>%ld</scale>%s</TrafficControlRequest>",(unsigned long)reqid, reqseq,scale,bounds_str);

	CloudSend(xml_str,url, base_req, method);
	//}


}

CARMACloudPlugin::~CARMACloudPlugin() {
}


string CARMACloudPlugin::updateTags(string str,string tagout, string tagin)
{
	int ind =0;
	while(1){
		ind = str.find(tagout,ind);
		if(ind!=string::npos) // did not have brackets, doesn't need them
		{
			str.replace(ind,tagout.length(),tagin);
		}
		else
			break;
	}	
	return str; 
}


void CARMACloudPlugin::CARMAResponseHandler(QHttpEngine::Socket *socket)
{
	QString st; 
	while(socket->bytesAvailable()>0)
	{	
		st.append(socket->readAll());
	}
	QByteArray array = st.toLocal8Bit();

	char* _cloudUpdate = array.data(); // would be the cloud update packet, needs parsing
	
	
	string tcm = _cloudUpdate;

    // new updateTags section
	tcm=updateTags(tcm,"<TrafficControlMessage>","<TestMessage05><body>");
	tcm=updateTags(tcm,"</TrafficControlMessage>","</body></TestMessage05>");
	tcm=updateTags(tcm,"TrafficControlParams","params");
	tcm=updateTags(tcm,"TrafficControlGeometry","geometry");
	tcm=updateTags(tcm,"TrafficControlPackage","package");

	
	tsm5Message tsm5message;
	tsm5EncodedMessage tsm5ENC;
	tmx::message_container_type container;
	std::unique_ptr<tsm5EncodedMessage> msg;


	std::stringstream ss;
	ss << tcm;  // updated _cloudUpdate tags, using updateTags

	container.load<XML>(ss);
	tsm5message.set_contents(container.get_storage().get_tree());
	tsm5ENC.encode_j2735_message(tsm5message);

	msg.reset();
	msg.reset(dynamic_cast<tsm5EncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE05_STRING)));

	string enc = tsm5ENC.get_encoding();
	msg->refresh_timestamp();
	msg->set_payload(tsm5ENC.get_payload_str());
	msg->set_encoding(enc);
	msg->set_flags(IvpMsgFlags_RouteDSRC);
	msg->addDsrcMetadata(172, 0x8003);
	msg->refresh_timestamp();

	routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
	BroadcastMessage(*rMsg);

	PLOG(logERROR) << " CARMACloud Plugin :: Broadcast tsm5:: " << tsm5ENC.get_payload_str();
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
	PLOG(logERROR)<<"CARMACloudPlugin:: Started web service";
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
	
	//std::string msg = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest><version>0.1</version><reqseq>99</reqseq><scale>0</scale><bounds><TrafficControlBounds><oldest>1584057600000</oldest><lon>-771521558</lon><lat>389504279</lat><xoffsets>10</xoffsets><xoffsets>20</xoffsets><xoffsets>10</xoffsets><yoffsets>0</yoffsets><yoffsets>500</yoffsets><yoffsets>500</yoffsets></TrafficControlBounds></bounds></TrafficControlRequest>";
	//std::string url ="http://127.0.0.1:22222";
	//std::string base = "/carmacloud/v2xhub";
	//std::string method = "POST";
	

	while (_plugin->state != IvpPluginState_error) {

		if (IsPluginState(IvpPluginState_registered))
		{
			//CloudSend(msg, url, base, method);
			this_thread::sleep_for(chrono::milliseconds(5000));
		}

	}

	return (EXIT_SUCCESS);
}
} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < CARMACloudPlugin::CARMACloudPlugin > ("CARMACloudPlugin", argc, argv);
}
