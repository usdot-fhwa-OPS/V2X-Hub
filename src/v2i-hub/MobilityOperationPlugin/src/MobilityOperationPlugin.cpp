//============================================================================
// Name        : MobilityOperationPlugin.cpp
// Author      : Paul Bourelly
// Version     : 5.0
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "MobilityOperationPlugin.h"


namespace MobilityOperationPlugin {



/**
 * Construct a new MobililtyOperationPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
MobilityOperationPlugin::MobilityOperationPlugin(string name) :
		PluginClient(name) {
		
	lock_guard<mutex> lock(_cfgLock);
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");
	//Set config values
	GetConfigValue<string>("Database_Username", _database_username);
    GetConfigValue<string>("Database_Password", _database_password);
	GetConfigValue<string>("Database_IP",_database_ip);
	GetConfigValue<uint16_t>("Database_Port",_database_port);
	AddMessageFilter < tsm3Message > (this, &MobilityOperationPlugin::HandleMobilityOperationMessage);
	SubscribeToMessages();
	std::string connection_string = "tcp://" + _database_ip + ":" + std::to_string(_database_port);
	try {
		driver = get_driver_instance();
		con = driver->connect(connection_string,_database_username,_database_password);
		con->setSchema("PORT_DRAYAGE");
	}
	catch ( sql::SQLException &e ) {
		FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
		FILE_LOG(logERROR) << "Error occurred in file " << __FILE__ << " on line " << __LINE__  << std::endl;
		FILE_LOG(logERROR) << "Error code " << e.getErrorCode() << std::endl;
		FILE_LOG(logERROR) << "Error status " << e.getSQLState() << std::endl;
	}
}

MobilityOperationPlugin::~MobilityOperationPlugin() {
}

void MobilityOperationPlugin::UpdateConfigSettings() {
	lock_guard<mutex> lock(_cfgLock);
	GetConfigValue<string>("Database_Username", _database_username);
    GetConfigValue<string>("Database_Password", _database_password);
	GetConfigValue<string>("Database_IP",_database_ip);
	GetConfigValue<uint16_t>("Database_Port",_database_port);
	std::string connection_string = "tcp://" + _database_ip + ":" + std::to_string(_database_port);
	try {
		driver = get_driver_instance();
		con = driver->connect(connection_string,_database_username,_database_password);
		con->setSchema("PORT_DRAYAGE");
	}
	catch ( sql::SQLException &e ) {
		FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
		FILE_LOG(logERROR) << "Error occurred in file " << __FILE__ << " on line " << __LINE__  << std::endl;
		FILE_LOG(logERROR) << "Error code " << e.getErrorCode() << std::endl;
		FILE_LOG(logERROR) << "Error status " << e.getSQLState() << std::endl;
	}
	
}

void MobilityOperationPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void MobilityOperationPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg ) {
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
			read_json(payload, pr);
			pd->cmv_id = (int) pr.get_child("cmv_id").get_value<int>();
			pd->cargo_id = pr.get_child("cargo_id").get_value<int>();
			pd->cargo =pr.get_child("cargo").get_value<bool>();
			pd->operation = pr.get_child("operation").get_value<std::string>();
			auto &location = pr.get_child("location");
			pd->location_long =location.get_child("longitude").get_value<float>();
			pd->location_lat = location.get_child("latitude").get_value<float>();
			auto &destination = pr.get_child("destination");
			pd->destination_long =destination.get_child("longitude").get_value<float>();
			pd->destination_lat = destination.get_child("latitude").get_value<float>();
			pd->action_id = pr.get_child("action_id").get_value<int>();
			pd->next_action = pr.get_child("next_action").get_value<int>();

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

	}
	try{
		stmt = con->createStatement();
		std::string queryString = "SELECT cmv_id, cargo_id, cargo, ST_X(location), ST_Y(location), ST_X(destination), ST_Y(destination), operation, action_id, next_action FROM freight WHERE action_ID = ";
		queryString.append(std::to_string(pd->next_action));
		res = stmt->executeQuery( queryString );
		while ( res->next() ) {
			FILE_LOG(logERROR) << "Column cmv_id: " << res->getString("cmv_id") << std::endl;
			FILE_LOG(logERROR) << "Column cargo_id: " << res->getString("cargo_id") << std::endl;
			FILE_LOG(logERROR) << "Column cargo: " << res->getString("cargo") << std::endl;
			FILE_LOG(logERROR) << "Column location: (" << res->getString("ST_X(location)") << ", " << res->getString("ST_Y(location)") << ")" << std::endl;
			FILE_LOG(logERROR) << "Column destination: (" << res->getString("ST_X(destination)") << ", " << res->getString("ST_Y(destination)") << ")" << std::endl;
			FILE_LOG(logERROR) << "Column operation: " << res->getString("operation") << std::endl;
			FILE_LOG(logERROR) << "Column action_id: " << res->getString("action_id") << std::endl;
			FILE_LOG(logERROR) << "Column next_action: " << res->getString("next_action") << std::endl;
		}
		
		// Initializer vars
		tsm3Message mob_msg;
		tsm3EncodedMessage mobilityENC;
		tmx::message_container_type container;
		std::unique_ptr<tsm3EncodedMessage> msg;

		//  Create operationParams payload json
		ptree payload;
		createPortDrayageJson( *pd, payload);
		std::stringstream pl;
		write_json( pl, payload);

		// Create XML MobilityOperationMessage
		ptree message;
		std::stringstream content;
		createMobilityOperationXml( message, payload );
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
	catch ( sql::SQLException &e ) {
		FILE_LOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl << e.what() << std::endl;
		FILE_LOG(logERROR) << "Error code " << e.getErrorCode() << std::endl;
		FILE_LOG(logERROR) << "Error status " << e.getSQLState() << std::endl;
	}


}

/**
 * Method to create port drayage payload JSON ptree using a PortDrayage_Object.
 * 
 * @param pd_obj Port Drayage object.
 * @param 
 */
void MobilityOperationPlugin::createPortDrayageJson( PortDrayage_Object &pd_obj, ptree &json_payload) {
	json_payload.put("cmv_id", pd_obj.cmv_id );
	json_payload.put("cargo_id", pd_obj.cargo_id );
	json_payload.put("cargo", pd_obj.cargo );
	ptree location;
	location.put("latitude", pd_obj.location_lat);
	location.put("longitude", pd_obj.location_long);
	ptree destination;
	destination.put("latitude", pd_obj.destination_lat);
	destination.put("longitude", pd_obj.destination_long);
	json_payload.put_child("destination",destination);
	json_payload.put("operation", pd_obj.operation );
	json_payload.put("action_id", pd_obj.action_id );
	json_payload.put("next_action", pd_obj.next_action );
}

/**
 * Method to create MobilityOperation XML ptree to allow for UPER encoding.
 */
void MobilityOperationPlugin::createMobilityOperationXml( ptree &mobilityOperationXml, ptree json_payload) {
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
	header.put("timestamp","0000000000000000000");
	message.put_child("header", header);
	message.put_child("body",body);
	mobilityOperationXml.put_child( "TestMessage03", message);
}

void MobilityOperationPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}


int MobilityOperationPlugin::Main() {
	FILE_LOG(logINFO) << "Starting plugin.";

	uint64_t lastSendTime = 0;

	while (_plugin->state != IvpPluginState_error) {
		


		usleep(100000); //sleep for microseconds set from config.
	}

	return (EXIT_SUCCESS);
}
}

int main(int argc, char *argv[]) {
	return run_plugin < MobilityOperationPlugin::MobilityOperationPlugin > ("MobilityOperationPlugin", argc, argv);
}

