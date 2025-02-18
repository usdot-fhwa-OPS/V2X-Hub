/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include "CDA1TenthPlugin.h"


namespace CDA1TenthPlugin {

CDA1TenthPlugin::CDA1TenthPlugin(string name) :
		PluginClient(name) {
	// Plugin Handles MobilityOperation Messages
	AddMessageFilter <tsm3Message> (this, &CDA1TenthPlugin::HandleMobilityOperationMessage);
	AddMessageFilter<BsmMessage>(this, &CDA1TenthPlugin::HandleBasicSafetyMessage);
	SubscribeToMessages();

	//Start websocket server
	std::thread wsThread(&CDA1TenthPlugin::startWebsocketServer, this);
	//Start UI message thread
	std::thread UIMessageThread(&CDA1TenthPlugin::startUIMessageThread, this);
	wsThread.join();
	UIMessageThread.join();
}

CDA1TenthPlugin::~CDA1TenthPlugin() {
	if(ws){
		ws->setRunning(false);		
	}
}

void CDA1TenthPlugin::UpdateConfigSettings() {

	lock_guard<mutex> lock(_cfgLock);
	std::string _database_username;
	std::string _database_password;
	uint16_t _database_port;
	std::string _database_ip;
	std::string _database_name;
	// Database configuration
	GetConfigValue<string>("Database_Username", _database_username);
	GetConfigValue<string>("Database_Password", _database_password);
	GetConfigValue<string>("Database_IP",_database_ip);
	GetConfigValue<uint16_t>("Database_Port",_database_port);
	GetConfigValue<string>("Database_Name", _database_name);
	// Get current strategy from V2X Hub config
	GetConfigValue<string>("Mobility_Strategy", _strat_config);
	uint16_t polling_frequency;
	uint16_t polling_timeout;
	std::string host;
	uint16_t port;
	bool secure;
	// Host address 
	GetConfigValue<string>("Webservice_Host", host);
	// Host Port
	GetConfigValue<uint16_t>("Webservice_Port", port);
	// True for HTTPS
	GetConfigValue<bool>("Webservice_Secure", secure);
	// Polling Frequency in seconds
	GetConfigValue<uint16_t>("Webservice_Polling_Frequency", polling_frequency);
	GetConfigValue<uint16_t>("BSM_Forward_Frequency", bsm_forward_frequency);

	// Create DB connection
	std::string connection_string = "tcp://" + _database_ip + ":" + std::to_string(_database_port);
	try {
		driver = get_driver_instance();
		con = driver->connect(connection_string,_database_username,_database_password);
		con->setSchema(_database_name);		
		//  Initialize PreparedStatements for MySQL, SQL preparedStatements for DB interface
		// Lookup/modify related actions
		first_action = con->prepareStatement("select * from action where action_id in (select min(action_id) from action where veh_id=?)");
		next_action_id = con->prepareStatement("SELECT next_action_id FROM action WHERE action_id = ?");
		current_action = con->prepareStatement("SELECT * FROM action WHERE action_id = ? ");
		prev_action_id = con->prepareStatement("SELECT prev_action_id FROM action WHERE action_id = ? ");
		insert_action =  con->prepareStatement("INSERT INTO action VALUES(?,?,?,?,?, UUID(), ?)");
		update_current_action = con->prepareStatement("UPDATE action SET next_action_id = ? WHERE action_id = ?");
		action_is_notify = con->prepareStatement("SELECT is_notify FROM action WHERE action_id = ?");

	}
	catch ( const sql::SQLException &e ) {
		PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl
			<< "Error occurred in file " << __FILE__ << " on line " << __LINE__  << std::endl
			<< "Error code " << e.getErrorCode() << std::endl
			<< "Error status " << e.getSQLState() << std::endl;
	}
	
}

void CDA1TenthPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void CDA1TenthPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg ) {
	// Retrieve J2735 Message
	auto mobilityOperation = msg.get_j2735_data();
	std::stringstream payload;
	std::stringstream strat_msg;
	// Create ptree object for json payload
	ptree jsonPayload;
	// Create new Action_Object pointer
	Action_Object actionObj;

	// Read strategy and operationParams
	strat_msg << mobilityOperation->body.strategy.buf;
	payload << mobilityOperation->body.operationParams.buf;
	//replace special characters &quot; with "
	auto payloadStr = payload.str();
	boost::replace_all(payloadStr, "&quot;", "\"");
	payload.str(payloadStr);

	// Compare strategy to CDA1Tenth strategy
	if ( _strat_config.compare(strat_msg.str()) == 0 ){
		try {
			PLOG(logINFO) << "Body OperationParams : " << mobilityOperation->body.operationParams.buf
				<< std::endl << "Body Strategy : " << mobilityOperation->body.strategy.buf << std::endl; 
			PLOG(logDEBUG) << "OperationParams payload : " << payload.str() << std::endl; 
			// Convert JSON payload to Action_Object
			read_json(payload, jsonPayload);
			actionObj = ActionConverter::fromTree( jsonPayload );
		}
		catch( const ptree_error &e ) {
			PLOG(logERROR) << "Error parsing Mobility Operation payload: " << e.what() << std::endl;
		}
	
		try{
			//Check if it is the first action
			if (actionObj.is_first_action) {
				PLOG(logDEBUG) << "Get first Action from database for vehicle id = " <<actionObj.vehicle.veh_id << std::endl;
				// Get first action
				actionObj = retrieveFirstAction(actionObj.vehicle.veh_id);	
				if(actionObj.action_id != INVALID_ACTION){
					printActionObject(actionObj);
					broadCastAction(actionObj, strat_msg.str());
					return;
				}				
			}
			else
			{
				PLOG(logDEBUG) << "Get current Action from database for action_id = " <<actionObj.action_id << ", vehicle id = " <<actionObj.vehicle.veh_id << std::endl;
				// Get current action
				actionObj = retrieveCurrentAction(actionObj.action_id);
				printActionObject(actionObj);
			}

			if(actionObj.action_id == INVALID_ACTION){
				PLOG(logERROR) << "Action id is invalid, abort further operation!" << std::endl;
				return;
			}

			//Check whether notify UI for further operation
			if(actionObj.area.is_notify){
				auto actionTree = ActionConverter::toTree(actionObj);
				string actionJsonString = BSMConverter::toJsonString(actionTree);
				PLOG(logDEBUG1) << "Add action object to websocket queue: " << actionJsonString;
				ws->addMessage(actionJsonString);
				PLOG(logDEBUG) << "Waiting for user (through UI) input on next action..." << std::endl;
			}else{
				PLOG(logDEBUG) << "Retrieving next action without waiting." << std::endl;
				auto nextAction = retrieveNextAction(actionObj.action_id );
				if(nextAction.action_id != INVALID_ACTION){
					printActionObject(nextAction);
					broadCastAction(nextAction, strat_msg.str());
				}
			}
		}
		catch ( const sql::SQLException &e ) {
			PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl
				<< "Error code " << e.getErrorCode() << std::endl
				<< "Error status " << e.getSQLState() << std::endl;
		}
	}
}

void CDA1TenthPlugin::broadCastAction(Action_Object &action_obj, const string &strategy){
	tsm3Message mob_msg;
	tsm3EncodedMessage mobilityENC;
	tmx::message_container_type container;
	std::unique_ptr<tsm3EncodedMessage> msg;

	//  Create operationParams payload json
	ptree payload = ActionConverter::toTree( action_obj );

	// Create XML MobilityOperationMessage
	ptree message = MobilityOperationConverter::toXML(payload, strategy);
	std::stringstream content;
	write_xml(content, message);				
	try {
		// Uper encode message 
		container.load<XML>(content);
		mob_msg.set_contents(container.get_storage().get_tree());
		mobilityENC.encode_j2735_message( mob_msg);
		msg.reset();
		msg.reset(dynamic_cast<tsm3EncodedMessage *>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE03_STRING)));
		string enc = mobilityENC.get_encoding();
		PLOG(logDEBUG) << "Encoded outgoing message : " << std::endl << mobilityENC.get_payload_str();
		msg->refresh_timestamp();
		msg->set_payload(mobilityENC.get_payload_str());
		msg->set_encoding(enc);
		msg->set_flags(IvpMsgFlags_RouteDSRC);
		msg->addDsrcMetadata(0xBFEE);
		msg->refresh_timestamp();
		routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
		BroadcastMessage(*rMsg);
	}
	catch ( const J2735Exception &e) {
		PLOG(logERROR) << "Error occurred during message encoding " << std::endl << e.what() << std::endl;
	}
}

void CDA1TenthPlugin::printActionObject(Action_Object &action_obj){
	PLOG(logDEBUG) << "Action Object : " << std::endl <<
		"action_id : " << action_obj.action_id << std::endl <<
		"next_action : " << action_obj.next_action << std::endl <<
		"prev_action : " << action_obj.prev_action << std::endl <<
		"area name : " << action_obj.area.name << std::endl <<
		std::fixed << std::setprecision(7) <<
		"area latitude : " << std::setw(10) << action_obj.area.latitude << std::endl <<
		"area longitude : " << std::setw(10) << action_obj.area.longitude << std::endl <<
		"area status : " << action_obj.area.status << std::endl <<
		"area is_notify : " << action_obj.area.is_notify << std::endl <<
		"cargo_uuid : " << action_obj.cargo.cargo_uuid << std::endl <<
		"cargo name : " << action_obj.cargo.name << std::endl <<
		"veh_id : " << action_obj.vehicle.veh_id << std::endl <<
		"vehicle name : " << action_obj.vehicle.name << std::endl;
}

Action_Object CDA1TenthPlugin::retrieveNextAction(const int &action_id ) {
	Action_Object rtn;
	try{
		// Set action_id
		next_action_id->setInt(1, action_id);
		// Get next action id
		sql::ResultSet *nextActionId = next_action_id->executeQuery();
		if ( nextActionId->first() ) {
			std::string next = nextActionId->getString("next_action_id");
			PLOG(logDEBUG) << "Next Action ID : " << next << std::endl;
			// Get next action
			rtn = retrieveCurrentAction( std::stoi(next) );
		}
		if(rtn.action_id == INVALID_ACTION){
			// If we are able to retrieve the current action but not it's next_action we can
			// assume it was the last action in the DB.
			PLOG(logINFO) << "Last action completed! No action found with next action id " << rtn.action_id << std::endl;
		}
	}
	catch ( const sql::SQLException &e ) {
		PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl
			<< "Error code " << e.getErrorCode() << std::endl
			<< "Error status " << e.getSQLState() << std::endl;
	}
	return rtn;
}

Action_Object CDA1TenthPlugin::retrieveCurrentAction(const int &action_id){
	Action_Object rtn;
	try {
		// Set action_id
		current_action->setInt(1, action_id);
		// Retrieve current action
		sql::ResultSet *cur_action = current_action->executeQuery();
		// Create Action_Object
		if (cur_action->first()) {
			rtn.action_id = cur_action->getInt("action_id");
			rtn.next_action = cur_action->getInt("next_action_id");
			rtn.prev_action = cur_action->getInt("prev_action_id");
			rtn.area.name = cur_action->getString("area_name");
			rtn.area.longitude = cur_action->getDouble("area_long");
			rtn.area.latitude = cur_action->getDouble("area_lat");
			rtn.area.status = cur_action->getString("area_status");
			rtn.area.is_notify = cur_action->getBoolean("area_is_notify");
			rtn.cargo.cargo_uuid = cur_action->getString("cargo_uuid");
			rtn.cargo.name = cur_action->getString("cargo_name");
			rtn.vehicle.veh_id = cur_action->getString("veh_id");
			rtn.vehicle.name = cur_action->getString("veh_name");
		} 
	} catch (const sql::SQLException &e) {
		PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl
			<< "Error code " << e.getErrorCode() << std::endl
			<< "Error status " << e.getSQLState() << std::endl;
	}
	return rtn;
}

Action_Object CDA1TenthPlugin::retrieveFirstAction(const string &vehicle_id){
	Action_Object rtn;
	try {
		// Set vehicle_id
		first_action->setString(1, vehicle_id);
		// Retrieve first action
		sql::ResultSet *first_action_rs = first_action->executeQuery();
		// Create Action_Object
		if (first_action_rs->first()) {
			rtn.action_id = first_action_rs->getInt("action_id");
			rtn.next_action = first_action_rs->getInt("next_action_id");
			rtn.prev_action = first_action_rs->getInt("prev_action_id");
			rtn.area.name = first_action_rs->getString("area_name");
			rtn.area.longitude = first_action_rs->getDouble("area_long");
			rtn.area.latitude = first_action_rs->getDouble("area_lat");
			rtn.area.status = first_action_rs->getString("area_status");
			rtn.area.is_notify = first_action_rs->getBoolean("area_is_notify");
			rtn.cargo.cargo_uuid = first_action_rs->getString("cargo_uuid");
			rtn.cargo.name = first_action_rs->getString("cargo_name");
			rtn.vehicle.veh_id = first_action_rs->getString("veh_id");
			rtn.vehicle.name = first_action_rs->getString("veh_name");
		} else {
			PLOG(logINFO) << "No first action found for vehicle id " << vehicle_id << std::endl;
		}
	} catch (const sql::SQLException &e) {
		PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl
			<< "Error code " << e.getErrorCode() << std::endl
			<< "Error status " << e.getSQLState() << std::endl;
	}
	return rtn;
}

void CDA1TenthPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg)
{
	receiveBasicSafetyMessage(msg);
}

void CDA1TenthPlugin::receiveBasicSafetyMessage(BsmMessage &msg)
{
	try
	{
		auto bsm = msg.get_j2735_data();
		auto bsmTree = BSMConverter::toTree(*bsm.get());
		auto bsmJsonString = BSMConverter::toJsonString(bsmTree);
		PLOG(logDEBUG2) << "Received BSM: " << bsmJsonString;

		//Add logic to control the BSM forwarding frequency 
		uint16_t bsm_forward_interval = 1000/bsm_forward_frequency; //Milliseconds
		uint64_t current_bsm_forward_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if(current_bsm_forward_time_ms - bsm_last_forward_time_ms >= bsm_forward_interval ){
				PLOG(logDEBUG1) << "Add BSM to websocket queue: " << bsmJsonString;
				ws->addMessage(bsmJsonString);
				bsm_last_forward_time_ms = current_bsm_forward_time_ms;
		}
	}
	catch (TmxException &ex)
	{
		PLOG(logERROR) << "Failed to decode message : " << ex.what();
		++_bsmMessageSkipped;
		SetStatus<uint>(Key_BSMMessageSkipped.c_str(), _bsmMessageSkipped);
	}
}

void CDA1TenthPlugin::startWebsocketServer()
{
	ws = std::make_shared<WebSocketServer>();
	ws->run();
}


void CDA1TenthPlugin::startUIMessageThread()
{
	while(_plugin->state != IvpPluginState_error){
		this_thread::sleep_for(chrono::milliseconds(1000));
		if(!ws->isRunning()){
			PLOG(logERROR) << "Websocket server is not running!" << std::endl;
			continue;
		}
		if(ws->receivedMessageQueueEmpty()){
			continue;
		}
		auto UIMessage = ws->popReceivedMessage();
		PLOG(logDEBUG1) << "Received UI message: " << UIMessage << std::endl;
		if(UIMessage.empty()){
			continue;
		}
		stringstream ss;
		ss << UIMessage;
		ptree jsonPayload;
		try{
			read_json(ss, jsonPayload);
			Action_Object actionObj = ActionConverter::fromTree( jsonPayload );
			//Retrieve next action to broadcast
			PLOG(logDEBUG) << "Retrieving next action for action_id = " << actionObj.action_id << std::endl;
			auto nextAction = retrieveNextAction(actionObj.action_id);
			if(nextAction.action_id != INVALID_ACTION){
				printActionObject(nextAction);
				broadCastAction(nextAction, _strat_config);
			}			
		}
		catch( const ptree_error &e ) {
			PLOG(logERROR) << "Error parsing UI message payload: " << e.what() << std::endl;
		}
	}
}
/**
 * Trigger when Plugin state changes
 * 
 * @param state IvpPluginState
 */ 
void CDA1TenthPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}


int CDA1TenthPlugin::Main() {
	uint64_t lastSendTime = 0;
	while (_plugin->state != IvpPluginState_error) {
		usleep(100000); //sleep for microseconds set from config.
	}
	return (EXIT_SUCCESS);
}
}

int main(int argc, char *argv[]) {
	// Qt HttpClient setup
	QCoreApplication a(argc, argv);
	return run_plugin < CDA1TenthPlugin::CDA1TenthPlugin > ("CDA1TenthPlugin", argc, argv);
}
