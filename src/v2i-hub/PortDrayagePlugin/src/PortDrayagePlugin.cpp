//============================================================================
// Name        : PortDrayagePlugin.cpp
// Author      : Paul Bourelly
// Version     : 5.0
// Copyright   : Your copyright notice
// Description : PortDrayagePlugin provides freight trucks in a port with a 
// list of actions to complete. On initial communication with V2X-Hub the 
// freight truck will request it's first action. Upon completion of each action
// the freight truck will send the completed action to V2X-Hub and the PortDrayagePlugin
// will retrieve it's next action from a MySQL DB.
//============================================================================

#include "PortDrayagePlugin.h"


namespace PortDrayagePlugin {




PortDrayagePlugin::PortDrayagePlugin(string name) :
		PluginClient(name) {
	
	AddMessageFilter < tsm3Message > (this, &PortDrayagePlugin::HandleMobilityOperationMessage);
	SubscribeToMessages();

}

PortDrayagePlugin::~PortDrayagePlugin() {
}

void PortDrayagePlugin::UpdateConfigSettings() {
	// Update configuration
	lock_guard<mutex> lock(_cfgLock);
	GetConfigValue<string>("Database_Username", _database_username);
	GetConfigValue<string>("Database_Password", _database_password);
	GetConfigValue<string>("Database_IP",_database_ip);
	GetConfigValue<uint16_t>("Database_Port",_database_port);
	GetConfigValue<string>("Database_Name", _database_name);
	GetConfigValue<string>("Web_Service_URL", _webservice_url);

	std::string loglevel;
	GetConfigValue<string>("LogLevel", loglevel);

	FILELog::ReportingLevel() = FILELog::FromString(loglevel);

	// Create DB connection
	std::string connection_string = "tcp://" + _database_ip + ":" + std::to_string(_database_port);
	try {
		driver = get_driver_instance();
		con = driver->connect(connection_string,_database_username,_database_password);
		con->setSchema(_database_name);
		
	}
	catch ( sql::SQLException &e ) {
		FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
		FILE_LOG(logERROR) << "Error occurred in file " << __FILE__ << " on line " << __LINE__  << std::endl;
		FILE_LOG(logERROR) << "Error code " << e.getErrorCode() << std::endl;
		FILE_LOG(logERROR) << "Error status " << e.getSQLState() << std::endl;
	}
	// Initialize PreparedStatements for MySQL
	try {
		next_action_id = con->prepareStatement("SELECT next_action FROM freight WHERE action_id = ? ");
		current_action = con->prepareStatement("SELECT * FROM freight WHERE action_id = ? ");
		first_action = con->prepareStatement("SELECT * FROM first_action WHERE cmv_id = ? " );
	}
	catch(std::exception &e) {
		FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
	}
}


void PortDrayagePlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void PortDrayagePlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg ) {

	// Retrieve J2735 Message
	auto mobilityOperation = msg.get_j2735_data();
	FILE_LOG(logERROR) << "Body OperationParams : " << mobilityOperation->body.operationParams.buf;
	FILE_LOG(logERROR) << "Body Strategy : " << mobilityOperation->body.strategy.buf;

	std::stringstream strat;
	std::stringstream payload; 
 
	ptree pr;
	PortDrayage_Object *pd = new PortDrayage_Object();
	strat << mobilityOperation->body.strategy.buf;
	payload << mobilityOperation->body.operationParams.buf;

	std::string strategy = strat.str();
	if ( strategy.compare(PORT_DRAYAGE_STRATEGY) == 0 ){
		try {
			// Convert JSON payload to PortDrayage_Object
			read_json(payload, pr);
			*pd = readPortDrayageJson( pr );
		}
		catch( const ptree_error &e ) {
			FILE_LOG(logERROR) << "Error parsing Mobility Operation payload: " << e.what() << std::endl;
		}
		FILE_LOG(logERROR) << "Port Drayage Message" << std::endl << 
			"cmv_id : " << pd->cmv_id << std::endl <<
			"cargo_id : " << pd->cargo_id << std::endl <<
			"cargo : " << pd->cargo << std::endl <<
			"operation : " << pd->operation << std::endl <<
			"location_lat : " << pd->location_lat << std::endl <<
			"location_long : " << pd->location_long << std::endl <<
			"destination_lat : " << pd->destination_lat << std::endl <<
			"destination_long : " << pd->destination_long << std::endl <<
			"action_id : " << pd->action_id << std::endl <<
			"next_action : " << pd->next_action << std::endl;

	
		try{
			// Retrieve first or next action
			PortDrayage_Object *new_action = new PortDrayage_Object();
			if ( pd->action_id.empty() ) {
				PLOG(logERROR) << "Retrieving first action." << std::endl;
				*new_action = retrieveFirstAction( pd->cmv_id );
			}
			else {
				PLOG(logERROR) << "Retrieving next action." << std::endl;
				*new_action = retrieveNextAction( pd->action_id );
			}

			if ( !new_action->action_id.empty()) {
				// Initializer vars
				tsm3Message mob_msg;
				tsm3EncodedMessage mobilityENC;
				tmx::message_container_type container;
				std::unique_ptr<tsm3EncodedMessage> msg;

				//  Create operationParams payload json
				ptree payload = createPortDrayageJson( *new_action );

				// Create XML MobilityOperationMessage
				ptree message = createMobilityOperationXml( payload );
				std::stringstream content;
				write_xml(content, message);

				FILE_LOG(logERROR) << "XML outgoing message : " << std::endl << content.str();
				
				try {
					// Uper encode message 
					container.load<XML>(content);
					mob_msg.set_contents(container.get_storage().get_tree());
					mobilityENC.encode_j2735_message( mob_msg);
					msg.reset();
					msg.reset(dynamic_cast<tsm3EncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE03_STRING)));
					string enc = mobilityENC.get_encoding();
					FILE_LOG(logERROR) << "Encoded outgoing message : " << std::endl << mobilityENC.get_payload_str();
					msg->refresh_timestamp();
					msg->set_payload(mobilityENC.get_payload_str());
					msg->set_encoding(enc);
					msg->set_flags(IvpMsgFlags_RouteDSRC);
					msg->addDsrcMetadata(172,0xBFEE);
					msg->refresh_timestamp();
					routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
					BroadcastMessage(*rMsg);
				}
				catch (J2735Exception &e) {
					FILE_LOG(logERROR) << "Error occurred during message encoding " << std::endl << e.what() << std::endl;
				}
			}
			else {
				FILE_LOG(logERROR) << "Could not find action!" << std::endl;
			}
			
		}
		catch ( sql::SQLException &e ) {
			FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
			FILE_LOG(logERROR) << "Error code " << e.getErrorCode() << std::endl;
			FILE_LOG(logERROR) << "Error status " << e.getSQLState() << std::endl;
		}
	}
}


ptree PortDrayagePlugin::createPortDrayageJson( PortDrayage_Object &pd_obj) {
	ptree json_payload;
	json_payload.put<int>("cmv_id", pd_obj.cmv_id );
	json_payload.put("cargo_id", pd_obj.cargo_id );
	ptree destination;
	destination.put<double>("latitude", pd_obj.destination_lat);
	destination.put<double>("longitude", pd_obj.destination_long);
	json_payload.put_child("destination",destination);
	json_payload.put("operation", pd_obj.operation );
	json_payload.put("action_id", pd_obj.action_id );
	return json_payload;
}


ptree PortDrayagePlugin::createMobilityOperationXml( ptree &json_payload ) {
	ptree mobilityOperationXml;
	std::stringstream pl;
	write_json( pl, json_payload);
	// Create XML MobilityOperationMessage
	ptree message;
	ptree header;
	ptree body;
	body.put("strategy",PORT_DRAYAGE_STRATEGY);
	body.put("operationParams", pl.str());
	header.put("hostStaticId", "UNSET");
	header.put("targetStaticId", "UNSET");
	header.put("hostBSMId", "00000000");
	header.put("planId", "00000000-0000-0000-0000-000000000000");
	header.put("timestamp", "0000000000000000000");
	message.put_child("header", header);
	message.put_child("body",body);
	mobilityOperationXml.put_child( "TestMessage03", message);
	return mobilityOperationXml;
}


PortDrayagePlugin::PortDrayage_Object PortDrayagePlugin::retrieveNextAction( std::string action_id ) {
	try{
		// Set action_id
		next_action_id->setString(1,action_id);
		// Get current action
		sql::ResultSet *cur_action = next_action_id->executeQuery();
		if ( cur_action->first() ) {
			std::string next = cur_action->getString("next_action");
			FILE_LOG(logERROR) << "Column next_action: " << next << std::endl;
			// Set action_id to next_action
			current_action->setString(1,next);
			// Retrieve next action
			sql::ResultSet *cur_action = current_action->executeQuery();
			
			// Create PortDrayage_Object
			PortDrayage_Object *rtn = new PortDrayage_Object();
			if ( cur_action->first() ) { 
				rtn->cmv_id = cur_action->getInt("cmv_id");
				rtn->operation = cur_action->getString("operation");
				rtn->action_id = cur_action->getString("action_id");
				rtn->cargo_id = cur_action->getString("cargo_id");
				rtn->destination_long = cur_action->getDouble("destination_long");
				rtn->destination_lat =  cur_action->getDouble("destination_lat");
				FILE_LOG(logERROR) << "Port Drayage Message : " << std::endl << 
					"cmv_id : " << rtn->cmv_id << std::endl <<
					"cargo_id : " << rtn->cargo_id << std::endl <<
					"operation : " << rtn->operation << std::endl <<
					"destination_lat : " << rtn->destination_lat << std::endl <<
					"destination_long : " << rtn->destination_long << std::endl <<
					"action_id : " << rtn->action_id << std::endl <<
					"next_action : " << rtn->next_action << std::endl;
			}
			else {
				// If we are able to retrieve the current action but not it's next_action we can
				// assume it was the last action in the DB.
				FILE_LOG(logERROR) << "Last action completed! No action found with action id " << next << std::endl;

			}
			return *rtn;
			

		}
		else {
			FILE_LOG(logERROR) << "No action with id : " << action_id << " found!";
			PortDrayage_Object *rtn = new PortDrayage_Object();
			return *rtn;
		}
		
	}
	catch ( sql::SQLException &e ) {
		FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
		FILE_LOG(logERROR) << "Error code " << e.getErrorCode() << std::endl;
		FILE_LOG(logERROR) << "Error status " << e.getSQLState() << std::endl;
	}
	
}

PortDrayagePlugin::PortDrayage_Object PortDrayagePlugin::retrieveFirstAction( uint32_t cmv_id ) {
	try{
		// Set cmv_id
		first_action->setInt(1,cmv_id);
		// Retrieve first action of first_action table
		sql::ResultSet *cur_action = first_action->executeQuery();
		if ( cur_action->first() ) {
			// Create Port Drayage object
			PortDrayage_Object *rtn = new PortDrayage_Object();
			rtn->cmv_id = cur_action->getInt("cmv_id");
			rtn->operation = cur_action->getString("operation");
			rtn->action_id = cur_action->getString("action_id");
			rtn->cargo_id = cur_action->getString("cargo_id");
			rtn->destination_long = cur_action->getDouble("destination_long");
			rtn->destination_lat =  cur_action->getDouble("destination_lat");
			rtn->next_action =cur_action->getString("next_action");
			FILE_LOG(logERROR) << "Port Drayage Message" << std::endl << 
				"cmv_id : " << rtn->cmv_id << std::endl <<
				"cargo_id : " << rtn->cargo_id << std::endl <<
				"operation : " << rtn->operation << std::endl <<
				"destination_lat : " << rtn->destination_lat << std::endl <<
				"destination_long : " << rtn->destination_long << std::endl <<
				"action_id : " << rtn->action_id << std::endl <<
				"next_action : " << rtn->next_action << std::endl;
			return *rtn;
		}
		else {
			PortDrayage_Object *rtn = new PortDrayage_Object();
			FILE_LOG(logERROR) << "No first action for cmv_id : " << cmv_id << " found!";
			return *rtn;
		}
		

	}
	catch ( sql::SQLException &e ) {
		FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
		FILE_LOG(logERROR) << "Error code " << e.getErrorCode() << std::endl;
		FILE_LOG(logERROR) << "Error status " << e.getSQLState() << std::endl;
	}
	
}


PortDrayagePlugin::PortDrayage_Object PortDrayagePlugin::readPortDrayageJson( ptree &pr ) {
	PortDrayage_Object *pd = new PortDrayage_Object();
	try {
		pd->cmv_id = pr.get_child("cmv_id").get_value<int>();
		boost::optional< ptree& > child = pr.get_child_optional( "action_id" );
		if( !child )
		{
			PLOG(logERROR) << "No action_id present! This is the vehicle's first action" << std::endl;
		}
		else {
			pd->action_id = child.get_ptr()->get_value<string>();
			// For eventually tracking of completed actions
			// pd->operation = pr.get_child("operation").get_value<string>();
			// ptree location = pr.get_child("location");
			// pd->location_lat =location.get_child("latitude").get_value<double>();
			// pd->location_long = location.get_child("longitude").get_value<double>();
			PLOG(logERROR) << "Action ID complete : " << pd->action_id << std::endl;

		}
		return *pd;

	}
	catch( const ptree_error &e ) {
		FILE_LOG(logERROR) << "Error parsing Mobility Operation payload: " << e.what() << std::endl;
	}
}

/**
 * Trigger when Plugin state changes
 * 
 * @param state IvpPluginState
 */ 
void PortDrayagePlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}


int PortDrayagePlugin::Main() {
	uint64_t lastSendTime = 0;
	while (_plugin->state != IvpPluginState_error) {
		usleep(100000); //sleep for microseconds set from config.
	}
	return (EXIT_SUCCESS);
}
}

int main(int argc, char *argv[]) {
	return run_plugin < PortDrayagePlugin::PortDrayagePlugin > ("PortDrayagePlugin", argc, argv);
}

