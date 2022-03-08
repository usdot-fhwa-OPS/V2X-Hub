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
		GetConfigValue<uint16_t>("fetchTime",fetchtime);
		GetConfigValue<string>("MobilityOperationStrategies", _strategies);	
		GetConfigValue<uint16_t>("TCMRepeatedlyBroadcastTimeOut",_TCMRepeatedlyBroadcastTimeOut);	
		GetConfigValue<string>("TCMNOAcknowledgementDescription", _TCMNOAcknowledgementDescription);
		AddMessageFilter < tsm4Message > (this, &CARMACloudPlugin::HandleCARMARequest);
		AddMessageFilter < tsm3Message > (this, &CARMACloudPlugin::HandleMobilityOperationMessage);

		// Subscribe to all messages specified by the filters above.
		SubscribeToMessages();
		std::thread webthread(&CARMACloudPlugin::StartWebService,this);
		webthread.detach(); // wait for the thread to finish 
		url ="http://127.0.0.1:33333"; // 33333 is the port that will send from v2xhub to carma cloud ## initally was 23665
		base_hb = "/carmacloud/v2xhub";
		base_req = "/carmacloud/tcmreq";
		method = "POST";
		_not_ACK_TCMs = std::make_shared<map<string, tsm5EncodedMessage>>();

}


void CARMACloudPlugin::HandleCARMARequest(tsm4Message &msg, routeable_message &routeableMsg)
{
	auto carmaRequest = msg.get_j2735_data();

	// create an XML template for the request
	//if(carmaRequest->body.present == TrafficControlRequest_PR_tcrV01) // taking this out since some message arent enabling this present variable. 
	// {

        // convert reqid bytes to hex string.
        size_t hexlen = 2; //size of each hex representation with a leading 0
        char reqid[carmaRequest->body.choice.tcrV01.reqid.size * hexlen + 1];
        for (int i = 0; i < carmaRequest->body.choice.tcrV01.reqid.size; i++)
        {
            sprintf(reqid+(i*hexlen), "%.2X", carmaRequest->body.choice.tcrV01.reqid.buf[i]);
        }

		printf("%s\n",reqid);

		long int reqseq = carmaRequest->body.choice.tcrV01.reqseq;
		long int scale = carmaRequest->body.choice.tcrV01.scale;
		

		int totBounds =  carmaRequest->body.choice.tcrV01.bounds.list.count;
		int cnt=0;
		char bounds_str[5000];
	       	strcpy(bounds_str,"");	
		
		//  get current time 
		std::time_t tm = std::time(0)/60-fetchtime*24*60; //  T minus 24 hours in  min  

		while(cnt<totBounds)
		{

			uint32_t oldest=tm;
		//	GetInt32((unsigned char*)carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->oldest.buf,&oldest);
			// = (int*)  carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->oldest.buf;
			long lat = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->reflat; 
			long longg = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->reflon;

		
			long dtx0 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[0]->deltax;
			long dty0 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[0]->deltay;
			long dtx1 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[1]->deltax;
			long dty1 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[1]->deltay;
			long dtx2 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[2]->deltax;
			long dty2 = carmaRequest->body.choice.tcrV01.bounds.list.array[cnt]->offsets.list.array[2]->deltay;

			sprintf(bounds_str+strlen(bounds_str),"<bounds><oldest>%ld</oldest><reflon>%ld</reflon><reflat>%ld</reflat><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets></bounds>",oldest,longg,lat,dtx0,dty0,dtx1,dty1,dtx2,dty2);

			cnt++;


		}

		char xml_str[10000]; 
        sprintf(xml_str,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest><reqid>%s</reqid><reqseq>%ld</reqseq><scale>%ld</scale>%s</TrafficControlRequest>",reqid, reqseq,scale,bounds_str);

	cout<<"Sent TCR: "<<xml_str<<endl;
	CloudSend(xml_str,url, base_req, method);
	//}
}

void CARMACloudPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg){
	std::vector<string> strategies_v;
	ConvertString2Vector(strategies_v, _strategies);

	//Process incoming MobilityOperation message
	auto mobilityOperationMsg = msg.get_j2735_data();
	PLOG(logERROR) << mobilityOperationMsg << std::endl;
	
	//process MobilityOperation strategy
	stringstream ss;
	ss << mobilityOperationMsg->body.strategy.buf;
	string mo_strategy = ss.str();
	std::transform(mo_strategy.begin(), mo_strategy.end(), mo_strategy.begin(), ::tolower );

	if( std::find( strategies_v.begin(), strategies_v.end(), mo_strategy ) != strategies_v.end() && mo_strategy.find("carma3/geofence_acknowledgement") != std::string::npos)
	{		
		//Process MobilityOperation trategy params
		ss.str("");
		ss << mobilityOperationMsg->body.operationParams.buf;
		string strategy_params_str = ss.str();	
		std::vector<string> strategy_params_v;
		ConvertString2Vector(strategy_params_v, strategy_params_str);	

		string even_log_description = GetValueFromStrategyParamsByKey(strategy_params_v, "reason");
		string acknnowledgement_str = GetValueFromStrategyParamsByKey(strategy_params_v, "acknowledgement");
		string traffic_control_id = GetValueFromStrategyParamsByKey(strategy_params_v, "traffic_control_id");
		boost::trim(traffic_control_id);//Trim white spaces
		std::transform(traffic_control_id.begin(), traffic_control_id.end(), traffic_control_id.begin(), ::tolower );	

		std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);
		//The traffic control id should match with the TCM id per CMV (CARMA vehicle).	
		if(_not_ACK_TCMs->erase(traffic_control_id) == 0)
		{
			PLOG(logERROR) << "Acknowledgement received, but traffic_control_id = " << traffic_control_id << " Not Found in TCM map." << std::endl;
		}
		
		//Create an event log object for both positive and negative ACK (ackownledgement), and broadcast the event log
		tmx::messages::TmxEventLogMessage event_log_msg;

		//acknnowledgement: Flag to indicate whether the received geofence was processed successfully by the CAV	
		std::transform(acknnowledgement_str.begin(), acknnowledgement_str.end(), acknnowledgement_str.begin(), ::tolower );	
		acknnowledgement_str.find("true") != std::string::npos ? event_log_msg.set_level(IvpLogLevel::IvpLogLevel_info) : event_log_msg.set_level(IvpLogLevel::IvpLogLevel_warn);
		event_log_msg.set_description(mo_strategy + ": " + even_log_description);
		PLOG(logERROR) << "event_log_msg " << event_log_msg << std::endl;
		this->BroadcastMessage<tmx::messages::TmxEventLogMessage>(event_log_msg);	
	}
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

void CARMACloudPlugin::removeTag(string& str,string openTag, string closeTag)
{
	int ind_open = 0;
	int ind_close = 0;
	while(ind_open != string::npos && ind_close !=string::npos)
	{
		ind_open = str.find(openTag, ind_open);
		ind_close = str.find(closeTag, ind_close);
		size_t len = ind_close - ind_open + strlen(closeTag.c_str());
		str.replace(ind_open, len, "");
	}
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

	cout<<"Received this from cloud"<<tcm<<endl;

    // new updateTags section
	tcm=updateTags(tcm,"<TrafficControlMessage>","<TestMessage05><body>");
	tcm=updateTags(tcm,"</TrafficControlMessage>","</body></TestMessage05>");
	tcm=updateTags(tcm,"TrafficControlParams","params");
	tcm=updateTags(tcm,"TrafficControlGeometry","geometry");
	tcm=updateTags(tcm,"TrafficControlPackage","package");
	cout<<"After update tag"<<tcm<<endl;

	removeTag(tcm, "<refwidth>", "</refwidth>");
	cout<<"After remove tag"<<tcm<<endl;
	
	tsm5Message tsm5message;
	tsm5EncodedMessage tsm5ENC;
	tmx::message_container_type container;


	std::stringstream ss;
	ss << tcm;  // updated _cloudUpdate tags, using updateTags

	container.load<XML>(ss);
	tsm5message.set_contents(container.get_storage().get_tree());
	tsm5ENC.encode_j2735_message(tsm5message);

	//Get TCM id
	Id64b_t tcmv01_req_id = tsm5message.get_j2735_data()->body.choice.tcmV01.reqid;
	ss.str(""); 
    for(size_t i=0; i < tcmv01_req_id.size; i++)
    {
       ss << std::hex << (unsigned) tcmv01_req_id.buf[i];
    }
	string tcmv01_req_id_hex = ss.str();	
	std::transform(tcmv01_req_id_hex.begin(), tcmv01_req_id_hex.end(), tcmv01_req_id_hex.begin(), ::tolower );	
	if(tcmv01_req_id_hex.length() > 0)
	{
		std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);
		_not_ACK_TCMs->insert({tcmv01_req_id_hex, tsm5ENC});
	}

	// std::thread t([this, tcmv01_id_hex , &tsm5ENC]()
	// {
	std::time_t start_time = 0, cur_time = 0;
	bool is_started_broadcasting = false; 
	
	while(_not_ACK_TCMs->count(tcmv01_req_id_hex) > 0 && ((cur_time - start_time) <= _TCMRepeatedlyBroadcastTimeOut) )
	{
		if(!is_started_broadcasting)
		{
			start_time = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
		}else{
			cur_time = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
		}
		is_started_broadcasting = true;
		PLOG(logERROR) << "cur_time= "<< cur_time << " start_time = "<< start_time  << "cur_time - start_time = " << cur_time - start_time << " _TCMRepeatedlyBroadcastTimeOut = " << _TCMRepeatedlyBroadcastTimeOut<< std::endl;
			
		if((cur_time - start_time) > _TCMRepeatedlyBroadcastTimeOut)
		{
			std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);
			PLOG(logERROR) << "CARMACloud Plugin does not receive ackownledgement within "<< _TCMRepeatedlyBroadcastTimeOut << " seconds. Time Out!" << std::endl;
			_not_ACK_TCMs->erase(tcmv01_req_id_hex);			
			break;
		}
		std::unique_ptr<tsm5EncodedMessage> msg;
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
		//sleep to control broadcast rate
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	// });//end of thread
	// t.join();
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
	GetConfigValue<uint16_t>("fetchTime",fetchtime);
	GetConfigValue<string>("MobilityOperationStrategies", _strategies);	
	GetConfigValue<uint16_t>("TCMRepeatedlyBroadcastTimeOut",_TCMRepeatedlyBroadcastTimeOut);	
	GetConfigValue<string>("TCMNOAcknowledgementDescription", _TCMNOAcknowledgementDescription);
	
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

void CARMACloudPlugin::ConvertString2Vector(std::vector<string> &sub_str_v, const string &str){
	stringstream ss;
	ss << str;
	while (ss.good())
	{
		std::string substring;
		getline(ss, substring, ',');		
		std::transform(substring.begin(), substring.end(),substring.begin(), ::tolower );
		sub_str_v.push_back( substring );
	}	
}

string CARMACloudPlugin::GetValueFromStrategyParamsByKey(const std::vector<string> & strategy_params_v, const string key)
{	
	string value = "";
	for(auto itr = strategy_params_v.begin();  itr != strategy_params_v.end(); itr++)
	{
		std::pair<string, string> key_value_pair;
		ConvertString2Pair(key_value_pair, *itr);
		if (key_value_pair.first.find(key) != std::string::npos)
		{
			value = key_value_pair.second;
			return value;
		}		
	}
	return value;		
}

void CARMACloudPlugin::ConvertString2Pair(std::pair<string,string> &str_pair, const string &str)
{
	stringstream ss;
	ss << str;
	size_t count = 0;
	string key, value;
	while (ss.good())
	{
		std::string substring;
		getline(ss, substring, ':');
		if(count == 0)
		{
			std::transform(substring.begin(), substring.end(),substring.begin(), ::tolower );
			key = substring;
			count += 1;
		}else{
			value = substring;
		}		
	}	
	str_pair = std::make_pair(key, value);
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
