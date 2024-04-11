#include "CARMACloudPlugin.h"
#include <WGS84Point.h>
#include <math.h>
#include <thread>
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
	base_hb = "/carmacloud/v2xhub";
	base_req = "/carmacloud/tcmreq";
	base_ack = "/carmacloud/tcmack";
	method = "POST";
	_not_ACK_TCMs = std::make_shared<multimap<string, tsm5EncodedMessage>>();
	_tcm_broadcast_times = std::make_shared<std::multimap<string, TCMBroadcastMetadata>>();
	_tcm_broadcast_starting_time = std::make_shared<std::map<string, std::time_t>>();
	std::thread Broadcast_t(&CARMACloudPlugin::TCMAckCheckAndRebroadcastTCM, this);
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

		sprintf(bounds_str+strlen(bounds_str),"<bounds><oldest>%u</oldest><reflon>%ld</reflon><reflat>%ld</reflat><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets><offsets><deltax>%ld</deltax><deltay>%ld</deltay></offsets></bounds>",oldest,longg,lat,dtx0,dty0,dtx1,dty1,dtx2,dty2);

		cnt++;


	}

	char xml_str[10000]; 
	sprintf(xml_str,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlRequest port=\"%s\" list=\"%s\"><reqid>%s</reqid><reqseq>%ld</reqseq><scale>%ld</scale>%s</TrafficControlRequest>",std::to_string(webport).c_str(),list_tcm.c_str(),reqid, reqseq,scale,bounds_str);

	PLOG(logINFO) << "Sent TCR to cloud: "<< xml_str<<endl;
	CloudSend(xml_str,carma_cloud_url, base_req, method);
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
		auto matching_TCMS = _not_ACK_TCMs->equal_range(traffic_control_id);
		bool is_tcm_removed = false;	
		for(auto itr = matching_TCMS.first; itr != matching_TCMS.second; itr++)
		{			
			//The traffic control id should match with the TCM id per CMV (CARMA vehicle) and combines with msgnum to uniquely identify each TCM.
			tsm5EncodedMessage msg = itr->second;
			tsm5Message decoded_tsm5_msg = msg.decode_j2735_message();
			std::shared_ptr<TestMessage05> msg_j2735_data = decoded_tsm5_msg.get_j2735_data();
			if (msg_j2735_data == NULL) {
				PLOG(logERROR) << "get_j2735_data() on decoded j2735 returned NULL." << std::endl;
				break;
			}

			if(msg_j2735_data->body.choice.tcmV01.msgnum == stol(msgnum))
			{				
				//Remove a single TCM identified by reqid (traffic control id) and msgnum.
				_not_ACK_TCMs->erase(itr);
				PLOG(logINFO) << "Acknowledgement received, traffic_control_id =" << traffic_control_id << ", msgnum = "<< msgnum << " removed from TCM map." << std::endl;
				is_tcm_removed = true;
				break;
			}
		}
		if(!is_tcm_removed)
		{
			PLOG(logERROR) << "Acknowledgement received, but traffic_control_id =" << traffic_control_id << ", msgnum = "<< msgnum << " NOT found in TCM map." << std::endl;
		}
		
		//Create an event log object for both positive and negative ACK (ackownledgement), and broadcast the event log
		tmx::messages::TmxEventLogMessage event_log_msg;

		//acknnowledgement: Flag to indicate whether the received geofence was processed successfully by the CMV. 1 mapping to acknowledged by CMV
		int ack = std::stoi(acknnowledgement_str);
		ack == acknowledgement_status::acknowledgement_status__acknowledged ? event_log_msg.set_level(IvpLogLevel::IvpLogLevel_info) : event_log_msg.set_level(IvpLogLevel::IvpLogLevel_warn);
		event_log_msg.set_description(mo_strategy + ": Traffic control id = " + traffic_control_id + ( CMV_id.length() <= 0 ? "":", CMV Id = " + CMV_id )+ ", reason = " + even_log_description);
		PLOG(logDEBUG) << "event_log_msg " << event_log_msg << std::endl;
		this->BroadcastMessage<tmx::messages::TmxEventLogMessage>(event_log_msg);	

		//Only send negative ack to carma-cloud if receiving any acks from CMV.
		if(ack != acknowledgement_status::acknowledgement_status__acknowledged  )
		{			
			stringstream sss;
			sss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlAcknowledgement><reqid> " << traffic_control_id
					<< "</reqid><msgnum>"<< msgnum 
					<<"</msgnum><cmvid>"<< CMV_id 
					<<"</cmvid><acknowledgement>" << acknowledgement_status::acknowledgement_status__rejected
					<< "</acknowledgement><description>" << even_log_description
					<< "</description></TrafficControlAcknowledgement>"; 
			PLOG(logINFO) << "Sent Negative ACK: "<< sss.str() <<endl;
			CloudSendAsync(sss.str(),carma_cloud_url, base_ack, method);
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
	QByteArray st; 
	while(socket->bytesAvailable()>0)
	{	
		auto readBytes = socket->readAll();
		st.append(readBytes);
	}

	if(st.size() == 0)
	{
		PLOG(logERROR) << "Received TCM is empty, and skipped." << std::endl;
		return;
	}
	PLOG(logINFO) << "Received TCM bytes size: " << st.size()<< std::endl;

	std::string tcm = "";
	bool isCompressed = socket->headers().keys().contains(CONTENT_ENCODING_KEY) && std::string(socket->headers().constFind(CONTENT_ENCODING_KEY).value().data()) == CONTENT_ENCODING_VALUE;
	if (isCompressed)
	{
		tcm = UncompressBytes(st).data();
	}else{
		tcm = st.data();		
	}

	//Transform carma-cloud TCM XML to J2735 compatible TCM XML by updating tags
	tcm=updateTags(tcm,"<TrafficControlMessage>","<TestMessage05><body>");
	tcm=updateTags(tcm,"</TrafficControlMessage>","</body></TestMessage05>");
	tcm=updateTags(tcm,"TrafficControlParams","params");
	tcm=updateTags(tcm,"TrafficControlGeometry","geometry");
	tcm=updateTags(tcm,"TrafficControlPackage","package");

	std::list<std::string> tcm_sl = {};
	if (isCompressed)
	{
		tcm_sl = FilterTCMs(tcm);
	}else{
		tcm_sl.push_back(tcm);
	}

	for(const auto tcm_s: tcm_sl)
	{
		tsm5Message tsm5message;
		tsm5EncodedMessage tsm5ENC;
		tmx::message_container_type container;

		std::stringstream ss;
		ss << tcm_s;

		container.load<XML>(ss);
		tsm5message.set_contents(container.get_storage().get_tree());
		tsm5ENC.encode_j2735_message(tsm5message);
		BroadcastTCM(tsm5ENC);
		PLOG(logINFO) << " CARMACloud Plugin :: Broadcast tsm5:: " << tsm5ENC.get_payload_str();

		//Get TCM id
		Id64b_t tcmv01_req_id = tsm5message.get_j2735_data()->body.choice.tcmV01.reqid;
		//Translate TCM request id to hex string
		ss.str(""); 
		for(size_t i=0; i < tcmv01_req_id.size; i++)
		{
			ss << std::setfill('0') << std::setw(2) << std::hex << (unsigned) tcmv01_req_id.buf[i];
		}
		string tcmv01_req_id_hex = ss.str();	
		//Transform all hex to lower case
		std::transform(tcmv01_req_id_hex.begin(), tcmv01_req_id_hex.end(), tcmv01_req_id_hex.begin(), ::tolower );	
		if(tcmv01_req_id_hex.length() > 0)
		{
			//Update map of tcm request id hex and tcm hex
			std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);
			_not_ACK_TCMs->insert({tcmv01_req_id_hex, tsm5ENC});
		}	
	}	
}

std::list<std::string> CARMACloudPlugin::FilterTCMs(const std::string& tcm_response) const
{
	std::list<std::string> tcm_sl = {};
	try
    {
        std::stringstream iss;
		iss << tcm_response; 
        boost::property_tree::ptree parent_node;
        boost::property_tree::read_xml(iss, parent_node);
        auto child_nodes = parent_node.get_child_optional("TrafficControlMessageList");
		//The tcm response is a list of TCM
        if (child_nodes)
        {
            for (const auto &p : child_nodes.get())
            {
                boost::property_tree::ptree tcm_node;
                tcm_node.put_child(p.first, p.second);
                std::ostringstream oss;
                boost::property_tree::write_xml(oss, tcm_node);
				tcm_sl.push_back(oss.str());
            }
        }else{
			tcm_sl.push_back(iss.str());
		}
    }
    catch (const boost::property_tree::xml_parser_error &e)
    {
        PLOG(logERROR) << "Failed to parse the xml string." << e.what();
    }
	return tcm_sl;
}

void CARMACloudPlugin::TCMAckCheckAndRebroadcastTCM()
{ 	
	while(true)
	{			
		std::this_thread::sleep_for(std::chrono::milliseconds(_TCMRepeatedlyBroadcastSleep));
		if(_plugin->state == IvpPluginState_error)
		{
			break;
		}
		if(_not_ACK_TCMs->size() > 0)
		{
			std::lock_guard<mutex> lock(_not_ACK_TCMs_mutex);

			std::set<string> expired_req_ids;

			for( auto itr = _not_ACK_TCMs->begin(); itr!=_not_ACK_TCMs->end(); itr++ )
			{
				string tcmv01_req_id_hex = itr->first;	
				auto cur_time = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();				
				if (_tcm_broadcast_starting_time->count(tcmv01_req_id_hex) == 0)
				{
					_tcm_broadcast_starting_time->insert({tcmv01_req_id_hex, cur_time});
				}
				else if ( (cur_time - _tcm_broadcast_starting_time->at(tcmv01_req_id_hex)) > _TCMRepeatedlyBroadcastTimeOut )
				{
					expired_req_ids.insert(tcmv01_req_id_hex);
					continue;
				}

				
				tsm5EncodedMessage tsm5ENC = itr->second;
				string tcm_hex_payload = tsm5ENC.get_payload_str();
				if(IsSkipBroadcastCurTCM(tcmv01_req_id_hex, tcm_hex_payload))
				{
					continue;
				}
				BroadcastTCM(tsm5ENC);
				PLOG(logINFO) << " CARMACloud Plugin :: Repeatedly Broadcast tsm5:: " << tsm5ENC.get_payload_str();
			} //END TCMs LOOP

			// For any ids which have expired clean up the maps
			for (auto tcmv01_req_id_hex : expired_req_ids) {
				//Create an event log object for both NO ACK (ackownledgement), and broadcast the event log
				tmx::messages::TmxEventLogMessage event_log_msg;
				event_log_msg.set_level(IvpLogLevel::IvpLogLevel_warn);
				event_log_msg.set_description(_TCMAcknowledgementStrategy + ": " + _TCMNOAcknowledgementDescription + " Traffic control id = " + tcmv01_req_id_hex);
				PLOG(logDEBUG) << "event_log_msg " << event_log_msg << std::endl;
				this->BroadcastMessage<tmx::messages::TmxEventLogMessage>(event_log_msg);	
				
				//send negative ack to carma-cloud
				stringstream sss;
				sss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TrafficControlAcknowledgement><reqid> " << tcmv01_req_id_hex
						<< "</reqid><msgnum></msgnum><cmvid></cmvid><acknowledgement>" <<  acknowledgement_status::acknowledgement_status__not_acknowledged
						<< "</acknowledgement><description>" << _TCMNOAcknowledgementDescription
						<< "</description></TrafficControlAcknowledgement>"; 
				PLOG(logINFO) << "Sent No ACK as Time Out: "<< sss.str() <<endl;
				CloudSendAsync(sss.str(),carma_cloud_url, base_ack, method);		

				_not_ACK_TCMs->erase(tcmv01_req_id_hex);
				//If time out, stop tracking the starting time of the TCMs being broadcast so far
				_tcm_broadcast_starting_time->erase(tcmv01_req_id_hex);
				//If time out, stop tracking the number of times the TCMs ( that has the same TCR reqid) being broadcast
				_tcm_broadcast_times->erase(tcmv01_req_id_hex);		
			}
		}
		else
		{
			PLOG(logDEBUG4) << "NO TCMs to broadcast." << std::endl;
			_tcm_broadcast_times->clear();
			_tcm_broadcast_starting_time->clear();
		}
	}
}

void CARMACloudPlugin::BroadcastTCM(tsm5EncodedMessage& tsm5ENC) {
	//Broadcast TCM
	string enc = tsm5ENC.get_encoding();
	std::unique_ptr<tsm5EncodedMessage> msg;
	msg.reset();
	msg.reset(dynamic_cast<tsm5EncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE05_STRING)));
	msg->refresh_timestamp();
	msg->set_payload(tsm5ENC.get_payload_str());
	msg->set_encoding(enc);
	msg->set_flags(IvpMsgFlags_RouteDSRC);
	msg->addDsrcMetadata(0x8003);
	msg->refresh_timestamp();
	routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
	BroadcastMessage(*rMsg);				
}

bool CARMACloudPlugin::IsSkipBroadcastCurTCM(const string & tcmv01_req_id_hex, const string & tcm_hex_payload ) const
{
	//Skip repeatedly broadcasting  
	if(_TCMRepeatedlyBroadCastTotalTimes == 0)
	{
			return true;
	}

	bool is_skip_cur_tcm = false;
	bool is_tcm_hex_found = false;
	auto tcms_metadatas = _tcm_broadcast_times->equal_range(tcmv01_req_id_hex);
	for(auto itr = tcms_metadatas.first; itr !=tcms_metadatas.second; itr ++)
	{
		string tcm_hex = itr->second.tcm_hex;
		int times = itr->second.num_of_times;
		if(tcm_hex == tcm_hex_payload)
		{
			is_tcm_hex_found = true;
			if (times >= _TCMRepeatedlyBroadCastTotalTimes)
			{						
				PLOG(logDEBUG) << "SKIP broadcasting as TCMs reqid = " << tcmv01_req_id_hex<< " has been repeatedly broadcast " << times  << " times." << std::endl;
				//Skip the broadcasting logic below if the TCMs with this request id has already been broadcast more than _TCMRepeatedlyBroadCastTotalTimes
				is_skip_cur_tcm = true;
			}
			else
			{
				//update the number of times a TCM being broadcast within time out period
				times += 1; //Increase by 1 for every iteration
				itr->second.num_of_times = times;
				PLOG(logDEBUG) << " TCMs reqid = " << tcmv01_req_id_hex<< " repeatedly broadcast " << times  << " times." << std::endl;
			}
		}
	}	

	if(!is_tcm_hex_found)
	{
		//Initialize the number of times a TCM being broadcast
		TCMBroadcastMetadata t;
		t.num_of_times = 1;
		t.tcm_hex = tcm_hex_payload;
		_tcm_broadcast_times->insert( {tcmv01_req_id_hex, t});
		PLOG(logDEBUG) << " TCMs reqid =  "<< tcmv01_req_id_hex << " has been broadcast once."<< std::endl;
	} 
	return is_skip_cur_tcm;
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
	PLOG(logINFO)<<"CARMACloudPlugin:: Started web service";
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
	GetConfigValue<int>("TCMRepeatedlyBroadCastTotalTimes", _TCMRepeatedlyBroadCastTotalTimes);
	GetConfigValue<int>("TCMRepeatedlyBroadcastSleep", _TCMRepeatedlyBroadcastSleep);
	GetConfigValue<string>("listTCM",list_tcm);
	std::string carma_cloud_ip;
	uint carma_cloud_port;
	GetConfigValue<string>("CARMACloudIP",carma_cloud_ip);
	GetConfigValue<uint>("CARMACloudPort",carma_cloud_port);
	carma_cloud_url = carma_cloud_ip + ":" + std::to_string(carma_cloud_port);
	PLOG(logDEBUG) << "Setting CARMA Cloud URL to " << carma_cloud_url << std::endl;
	
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

void CARMACloudPlugin::CloudSendAsync(const string& local_msg,const string& local_url, const string& local_base, const string& local_method)
{
	std::thread t([this, &local_msg, &local_url, &local_base, &local_method](){	
		CloudSend(local_msg, local_url, local_base, local_method);	
	});
	t.detach();
}

int CARMACloudPlugin::CloudSend(const string &local_msg, const string& local_url, const string& local_base, const string& local_method)
{ 	
	CURL *req;
	CURLcode res;
	string urlfull = local_url+local_base;	
	req = curl_easy_init();
	if(req) {
		curl_easy_setopt(req, CURLOPT_URL, urlfull.c_str());

		if(strcmp(local_method.c_str(),"POST")==0)
		{
			curl_easy_setopt(req, CURLOPT_POSTFIELDS, local_msg.c_str());
			curl_easy_setopt(req, CURLOPT_TIMEOUT_MS, 1000L); // Request operation complete within max millisecond timeout 
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
			value +=  substring + " ";
		}		
	}	
	str_pair = std::make_pair(key, value);
}

QByteArray CARMACloudPlugin::UncompressBytes(const QByteArray compressedBytes) const
{
    z_stream strm;
	strm.zalloc = Z_NULL;//Refer to zlib docs (https://zlib.net/zlib_how.html)
	strm.zfree = Z_NULL; 
    strm.opaque = Z_NULL;
    strm.avail_in = compressedBytes.size();
	strm.next_in = (Byte *)compressedBytes.data();
	//checking input z_stream to see if there is any error, eg: invalid data etc.
    auto err = inflateInit2(&strm, MAX_WBITS+32); // gzip input https://stackoverflow.com/questions/1838699/how-can-i-decompress-a-gzip-stream-with-zlib
    QByteArray outBuf;
	//MAX numbers of bytes stored in a buffer 
    const int BUFFER_SIZE = 4092;
	//There is successful, starting to decompress data
    if (err == Z_OK) 
    {
        int isDone = 0;
        do
        {
            char buffer[BUFFER_SIZE] = {0};
            strm.avail_out = BUFFER_SIZE;
            strm.next_out = (Byte *)buffer;
            isDone = inflate(&strm, Z_NO_FLUSH);
			outBuf.append(buffer, BUFFER_SIZE - strm.avail_out);
		} while (Z_STREAM_END != isDone); // Reach the end of stream to be uncompressed
	}else{
		PLOG(logWARNING) << "Error initalize stream. Err code = " << err << std::endl;
	}
	//Finished decompress data stream
    inflateEnd(&strm);
    return outBuf;
}

int CARMACloudPlugin::Main() {


	FILE_LOG(logINFO) << "Starting plugin.";		

	while (_plugin->state != IvpPluginState_error) {

		if (IsPluginState(IvpPluginState_registered))
		{
			this_thread::sleep_for(chrono::milliseconds(5000));
		}
	}

	return (EXIT_SUCCESS);
}
} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < CARMACloudPlugin::CARMACloudPlugin > ("CARMACloudPlugin", argc, argv);
}
