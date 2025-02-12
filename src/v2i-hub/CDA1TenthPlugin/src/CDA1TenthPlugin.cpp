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

}

CDA1TenthPlugin::~CDA1TenthPlugin() {
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
	// TODO Update for new Web Service Configuration
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

	// TODO: CHECK PREPARED STATEMENTS AGAINST FINAL DATABASE
	// Create DB connection
	std::string connection_string = "tcp://" + _database_ip + ":" + std::to_string(_database_port);
	try {
		driver = get_driver_instance();
		con = driver->connect(connection_string,_database_username,_database_password);
		con->setSchema(_database_name);
		// Initialize PreparedStatements for MySQL
		// Get next_action for given action_id
		
		// SQL preparedStatements for DB interface
		// Lookup/modify related actions
		next_action_id = con->prepareStatement("SELECT next_action FROM action WHERE veh_id = ? and action_id = ?");
		current_action = con->prepareStatement("SELECT * FROM action WHERE veh_id = ? and action_id = ? ");
		prev_action_id = con->prepareStatement("SELECT prev_action FROM action WHERE veh_id = ? and action_id = ? ");
		first_action = con->prepareStatement("SELECT * FROM action WHERE veh_id = ? and action_id = 0 " );
		insert_action =  con->prepareStatement("INSERT INTO freight VALUES(?,?,?,?,?, UUID(), ?)");
		update_current_action = con->prepareStatement("UPDATE freight SET next_action = ? WHERE veh_id = ? and action_id = ?");
		// Retrieve data for a given action_id
		action_is_notify = con->prepareStatement("SELECT is_notify FROM action WHERE veh_id = ? and action_id = ?");

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

	// String Stream for strategy and operationParams
	std::string strat_config;
	std::stringstream payload;
	std::stringstream strat_msg;

	// Get current strategy from V2X Hub config
	GetConfigValue<string>("Mobility_Strategy", strat_config);

	// Create ptree object for json payload
	ptree json_payload;

	// Create new Action_Object pointer
	std::unique_ptr<Action_Object> action_obj( new Action_Object());

	// Read strategy and operationParams
	strat_msg << mobilityOperation->body.strategy.buf;
	payload << mobilityOperation->body.operationParams.buf;

	// // Compare strategy to CDA1Tenth strategy
	if ( strat_config.compare(strat_msg.str()) == 0 ){
		try {
			PLOG(logINFO) << "Body OperationParams : " << mobilityOperation->body.operationParams.buf
				<< std::endl << "Body Strategy : " << mobilityOperation->body.strategy.buf; 
			// Convert JSON payload to Action_Object
			read_json(payload, json_payload);
			*action_obj = ActionConverter::fromTree( json_payload );
		}
		catch( const ptree_error &e ) {
			PLOG(logERROR) << "Error parsing Mobility Operation payload: " << e.what() << std::endl;
		}

		PLOG(logDEBUG) << "Generated Action Object : " << std::endl << 
			"action_id : " << action_obj->action_id << std::endl <<
			"next_action : " << action_obj->next_action << std::endl <<
			"prev_action : " << action_obj->prev_action << std::endl <<
			"area name : " << action_obj->area.name << std::endl <<
			"area latitude : " << action_obj->area.latitude << std::endl <<
			"area longitude : " << action_obj->area.longitude << std::endl <<
			"area status : " << action_obj->area.status << std::endl <<
			"area is_notify : " << action_obj->area.is_notify << std::endl <<
			"cargo_uuid : " << action_obj->cargo.cargo_uuid << std::endl <<
			"cargo name : " << action_obj->cargo.name << std::endl << 
			"veh_id : " << action_obj->vehicle.veh_id << std::endl <<
			"vehicle name : " << action_obj->vehicle.name << std::endl;
	
		try{
			Action_Object *new_action = new Action_Object();

			if (action_obj->action_id == -1 ) {
				PLOG(logERROR) << "Action id is invalid" << std::endl;
			}
			else {
				if (action_obj->area.is_notify == false){
					PLOG(logDEBUG) << "Retrieving next action without waiting." << std::endl;
					*new_action = retrieveNextAction(action_obj->action_id );
				}
				else {
					PLOG(logDEBUG) << "Waiting for user input on next action" << std::endl;

					ptree action_tree = ActionConverter::toTree(*action_obj);
					string action_string = BSMConverter::toJsonString(action_tree);
				
					// TODO: NEED TO REASSESS USE OF OPERATION CODES DEFINED IN HEADER. PICKUP, DROPOFF, ETC. CAN POTENTIALLY BE MORE GENERIC.
					//		THE CURRENT ENUM/SWITCH CASE IS NOT VERY EXTENSIBLE OR REUSABLE ACROSS USE CASES DUE TO STRING MATCHING IN DB.

					// // forward current action to the UI in json format as a request
					// 	client->request_cargo_update(action_string); //push to message queue
					// }
					// 	client->request_action_queue_update(action_string;
					// }

					// request that a conencted UI action is performed after UI is processed
					// check msg queue on a thread and trigger
				}
			}

			// make function
			if (new_action->action_id != -1) {
				// Initializer vars
				tsm3Message mob_msg;
				tsm3EncodedMessage mobilityENC;
				tmx::message_container_type container;
				std::unique_ptr<tsm3EncodedMessage> msg;

				//  Create operationParams payload json
				ptree payload = ActionConverter::toTree( *new_action );

				// Create XML MobilityOperationMessage
				ptree message = MobilityOperationConverter::toXML(payload, strat_msg.str());
				std::stringstream content;
				write_xml(content, message);				
				try {
					// Uper encode message 
					container.load<XML>(content);
					mob_msg.set_contents(container.get_storage().get_tree());
					mobilityENC.encode_j2735_message( mob_msg);
					msg.reset();
					msg.reset(dynamic_cast<tsm3EncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE03_STRING)));
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
			else {
				PLOG(logWARNING) << "Could not find valid action!" << std::endl;
			}
			
		}
		catch ( const sql::SQLException &e ) {
			PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl
				<< "Error code " << e.getErrorCode() << std::endl
				<< "Error status " << e.getSQLState() << std::endl;
		}
	}
}

Action_Object CDA1TenthPlugin::retrieveNextAction(const int &action_id ) {
	std::unique_ptr<Action_Object> rtn( new Action_Object());
	try{
		// Set action_id
		next_action_id->setInt(1,action_id);
		// Get current action
		sql::ResultSet *cur_action = next_action_id->executeQuery();
		if ( cur_action->first() ) {
			std::string next = cur_action->getString("next_action");
			// Set action_id to next_action
			current_action->setString(1,next);
			// Retrieve next action
			sql::ResultSet *cur_action = current_action->executeQuery();
			// Create Action_Object
			if ( cur_action->first() ) { 
				rtn->action_id = cur_action->getInt("action_id");
				rtn->next_action = cur_action->getInt("next_action");
				rtn->prev_action = cur_action->getInt("prev_action");
				rtn->area.name = cur_action->getString("area_name");
				rtn->area.longitude = cur_action->getDouble("area_long");
				rtn->area.latitude =  cur_action->getDouble("area_lat");
				rtn->area.status = cur_action->getBoolean("area_status");
				rtn->area.is_notify = cur_action->getBoolean("area_is_notify");
				rtn->cargo.cargo_uuid = cur_action->getString("cargo_uuid");
				rtn->cargo.name = cur_action->getString("cargo_name");
				rtn->vehicle.veh_id = cur_action->getString("veh_id");
				rtn->vehicle.name = cur_action->getString("veh_name");

				PLOG(logDEBUG) << "Mobility Operation Message : " << std::endl << 
					"action_id : " << rtn->action_id << std::endl <<
					"next_action : " << rtn->next_action << std::endl <<
					"prev_action : " << rtn->prev_action << std::endl <<
					"area name : " << rtn->area.name << std::endl <<
					"area latitude : " << rtn->area.latitude << std::endl <<
					"area longitude : " << rtn->area.longitude << std::endl <<
					"area status : " << rtn->area.status << std::endl <<
					"area is_notify : " << rtn->area.is_notify << std::endl <<
					"cargo_uuid : " << rtn->cargo.cargo_uuid << std::endl <<
					"cargo name : " << rtn->cargo.name << std::endl << 
					"veh_id : " << rtn->vehicle.veh_id << std::endl <<
					"vehicle name : " << rtn->vehicle.name << std::endl;
			}
			else {
				// If we are able to retrieve the current action but not it's next_action we can
				// assume it was the last action in the DB.
				PLOG(logINFO) << "Last action completed! No action found with action id " << next << std::endl;

			}
		}
	}
	catch ( const sql::SQLException &e ) {
		PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl
			<< "Error code " << e.getErrorCode() << std::endl
			<< "Error status " << e.getSQLState() << std::endl;
	}
	return *rtn;
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
			PLOG(logDEBUG) << "Received BSM: " << bsmJsonString;
		}
		catch (TmxException &ex)
		{
			PLOG(logERROR) << "Failed to decode message : " << ex.what();
			++_bsmMessageSkipped;
			SetStatus<uint>(Key_BSMMessageSkipped.c_str(), _bsmMessageSkipped);
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
