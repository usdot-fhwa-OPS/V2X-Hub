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

	UpdateConfigSettings();
	std::lock_guard<mutex> lock(_cfgLock);
	AddMessageFilter < tsm4Message > (this, &CARMACloudPlugin::HandleCARMARequest);
	AddMessageFilter < tsm3Message > (this, &CARMACloudPlugin::HandleMobilityOperationMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
	std::thread webthread(&CARMACloudPlugin::StartWebService,this);
	webthread.detach(); // wait for the thread to finish 
	url ="http://127.0.0.1:33333"; // 33333 is the port that will send from v2xhub to carma cloud ## initally was 23665
	base_hb = "/carmacloud/v2xhub";
	base_req = "/carmacloud/tcmreq";
	base_ack = "/carmacloud/tcmack";
	method = "POST";
	_not_ACK_TCMs = std::make_shared<multimap<string, tsm5EncodedMessage>>();
	std::thread Broadcast_t(&CARMACloudPlugin::Broadcast_TCMs, this);
	Broadcast_t.detach();

}


void CARMACloudPlugin::HandleCARMARequest(tsm4Message &msg, routeable_message &routeableMsg)
{
	auto carmaRequest = msg.get_j2735_data();



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

	PLOG(logINFO) << "Sent TCR: "<< xml_str<<endl;
	CloudSend(xml_str,url, base_req, method);

}

void CARMACloudPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg){
	std::vector<string> strategies_v;
	ConvertString2Vector(strategies_v, _strategies);

	//Process incoming MobilityOperation message
	auto mobilityOperationMsg = msg.get_j2735_data();
	
	//process MobilityOperation strategy 
	stringstream ss;
	ss << mobilityOperationMsg->body.strategy.buf;
	string mo_strategy = ss.str();
	std::transform(mo_strategy.begin(), mo_strategy.end(), mo_strategy.begin(), ::tolower );

	if( std::find( strategies_v.begin(), strategies_v.end(), mo_strategy ) != strategies_v.end() && mo_strategy.find(_TCMAcknowledgementStrategy) != std::string::npos)
	{		
		//Process MobilityOperation strategy params
		ss.str("");
		ss << mobilityOperationMsg->body.operationParams.buf;
		string strategy_params_str = ss.str();	
		std::vector<string> strategy_params_v;
		ConvertString2Vector(strategy_params_v, strategy_params_str);	

		string even_log_description = GetValueFromStrategyParamsByKey(strategy_params_v, "reason");
		string acknnowledgement_str = GetValueFromStrategyParamsByKey(strategy_params_v, "acknowledgement");
		string traffic_control_id = GetValueFromStrategyParamsByKey(strategy_params_v, "traffic_control_id");
		string msgnum = GetValueFromStrategyParamsByKey(strategy_params_v, "msgnum");
		boost::trim(traffic_control_id);
		std::transform(traffic_control_id.begin(), traffic_control_id.end(), traffic_control_id.begin(), ::tolower );	
		ss.str("");
		ss << mobilityOperationMsg->header.hostStaticId.buf;
		string CMV_id = ss.str();

		std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);
		//The traffic control id should match with the TCM id per CMV (CARMA vehicle).	
		if(_not_ACK_TCMs->erase(traffic_control_id) <= 0)
		{
			PLOG(logERROR) << "Acknowledgement received, but traffic_control_id =" << traffic_control_id << " Not Found in TCM map." << std::endl;
		}
		
		//Create an event log object for both positive and negative ACK (ackownledgement), and broadcast the event log
		tmx::messages::TmxEventLogMessage event_log_msg;

		//acknnowledgement: Flag to indicate whether the received geofence was processed successfully by the CMV. 1 mapping to acknowledged by CMV
		std::transform(acknnowledgement_str.begin(), acknnowledgement_str.end(), acknnowledgement_str.begin(), ::tolower );	
		acknnowledgement_str.find("1") != std::string::npos ? event_log_msg.set_level(IvpLogLevel::IvpLogLevel_info) : event_log_msg.set_level(IvpLogLevel::IvpLogLevel_warn);
		event_log_msg.set_description(mo_strategy + ": Traffic control id = " + traffic_control_id + ( CMV_id.length() <= 0 ? "":", CMV Id = " + CMV_id )+ ", reason = " + even_log_description);
		PLOG(logDEBUG) << "event_log_msg " << event_log_msg << std::endl;
		this->BroadcastMessage<tmx::messages::TmxEventLogMessage>(event_log_msg);	

		//send negative ack to carma-cloud if not receiving any ack from CMV. acknowledgement_status__acknowledged	= 1	
		if(acknnowledgement_str.find("1") == std::string::npos )
		{			
			char xml_str[10000]; 
			sprintf(xml_str,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlAcknowledgement><reqid>%s</reqid><msgnum>%s</msgnum><cmvid>%s</cmvid><acknowledgement>%d</acknowledgement><description>%s</description></TrafficControlAcknowledgement>",traffic_control_id.c_str(), msgnum.c_str() ,CMV_id.c_str(),acknowledgement_status::acknowledgement_status__rejected,even_log_description);
			PLOG(logINFO) << "Sent Negative ACK: "<< xml_str<<endl;
			CloudSend(xml_str,url, base_ack, method);
		}
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

	PLOG(logINFO) << "Received this from cloud" << tcm << std::endl;
	if(tcm.length() == 0)
	{
		PLOG(logERROR) << "Received TCM length is zero, and skipped." << std::endl;
		return;
	}
    // new updateTags section
	tcm=updateTags(tcm,"<TrafficControlMessage>","<TestMessage05><body>");
	tcm=updateTags(tcm,"</TrafficControlMessage>","</body></TestMessage05>");
	tcm=updateTags(tcm,"TrafficControlParams","params");
	tcm=updateTags(tcm,"TrafficControlGeometry","geometry");
	tcm=updateTags(tcm,"TrafficControlPackage","package");
	
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
		ss << std::setfill('0') << std::setw(2) << std::hex << (unsigned) tcmv01_req_id.buf[i];
    }
	string tcmv01_req_id_hex = ss.str();	
	
	std::transform(tcmv01_req_id_hex.begin(), tcmv01_req_id_hex.end(), tcmv01_req_id_hex.begin(), ::tolower );	
	if(tcmv01_req_id_hex.length() > 0)
	{
		std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);
		_not_ACK_TCMs->insert({tcmv01_req_id_hex, tsm5ENC});
	}	
}

void CARMACloudPlugin::Broadcast_TCMs()
{ 	
	std::time_t start_time = 0, cur_time = 0;
	bool is_started_broadcasting = false;
	while(true)
	{			
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if(_plugin->state == IvpPluginState_error)
		{
			break;
		}
		if(_not_ACK_TCMs->size() > 0)
		{
			std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);
			for( auto itr = _not_ACK_TCMs->begin(); itr!=_not_ACK_TCMs->end(); ++itr )
			{
				string tcmv01_req_id_hex = itr->first;
				if(!is_started_broadcasting)
				{
					start_time = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
				}else{
					cur_time = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
				}
				is_started_broadcasting = true;
				if((cur_time - start_time) > _TCMRepeatedlyBroadcastTimeOut)
				{
					_not_ACK_TCMs->erase(tcmv01_req_id_hex);
					start_time = 0;
					cur_time = 0;	
					is_started_broadcasting = false;

					//Create an event log object for both NO ACK (ackownledgement), and broadcast the event log
					tmx::messages::TmxEventLogMessage event_log_msg;
					event_log_msg.set_level(IvpLogLevel::IvpLogLevel_warn);
					event_log_msg.set_description(_TCMAcknowledgementStrategy + ": " + _TCMNOAcknowledgementDescription + " Traffic control id = " + tcmv01_req_id_hex);
					PLOG(logDEBUG) << "event_log_msg " << event_log_msg << std::endl;
					this->BroadcastMessage<tmx::messages::TmxEventLogMessage>(event_log_msg);	

					//send negative ack to carma-cloud
					char xml_str[10000]; 
					sprintf(xml_str,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlAcknowledgement><reqid>%s</reqid><msgnum>%s</msgnum><cmvid>%s</cmvid><acknowledgement>%d</acknowledgement><description>%s</description></TrafficControlAcknowledgement>",tcmv01_req_id_hex.c_str(), "", "", acknowledgement_status::acknowledgement_status__not_acknowledged,_TCMNOAcknowledgementDescription.c_str());
					PLOG(logINFO) << "Sent No ACK as Time Out: "<< xml_str<<endl;
					CloudSend(xml_str,url, base_ack, method);					
					break;
				}
				std::unique_ptr<tsm5EncodedMessage> msg;
				msg.reset();
				msg.reset(dynamic_cast<tsm5EncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE05_STRING)));
				tsm5EncodedMessage tsm5ENC = itr->second;
				string enc = tsm5ENC.get_encoding();
				msg->refresh_timestamp();
				msg->set_payload(tsm5ENC.get_payload_str());
				msg->set_encoding(enc);
				msg->set_flags(IvpMsgFlags_RouteDSRC);
				msg->addDsrcMetadata(172, 0x8003);
				msg->refresh_timestamp();

				routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
				BroadcastMessage(*rMsg);		
				PLOG(logINFO) << " CARMACloud Plugin :: Broadcast tsm5:: " << tsm5ENC.get_payload_str();
			}
		}
		else
		{
			start_time = 0;
			cur_time = 0;	
			is_started_broadcasting = false;
		}
	}
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
    std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue<uint64_t>("Frequency", _frequency);	
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
			curl_easy_setopt(req, CURLOPT_TIMEOUT, 2L);
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

void CARMACloudPlugin::ConvertString2Vector(std::vector<string> &sub_str_v, const string &str) const{
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

string CARMACloudPlugin::GetValueFromStrategyParamsByKey(const std::vector<string> & strategy_params_v, const string& key) const
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

void CARMACloudPlugin::ConvertString2Pair(std::pair<string,string> &str_pair, const string &str) const
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
