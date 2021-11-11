#ifndef SRC_PortDrayagePlugin_H_
#define SRC_PortDrayagePlugin_H_
#include "PluginClient.h"
#include <tmx/j2735_messages/testMessage03.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <OAIDefaultApi.h>
#include <QEventLoop>
#include <QTimer>
#include <OAIHelpers.h>
#include <QCoreApplication>
#include "WebServiceClient.h"





using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace boost::property_tree;
using namespace OpenAPI;




namespace PortDrayagePlugin {
static CONSTEXPR const char *PORT_DRAYAGE_STRATEGY = "carma/port_drayage";

class PortDrayagePlugin: public PluginClient {
public:
	struct PortDrayage_Object {
		std::string cmv_id; 
		std::string cargo_id;
		bool cargo;
		std::string operation;
		double location_lat;
		double location_long;
		double destination_lat;
		double destination_long;
		std::string action_id;
		std::string next_action;
	};
	/**
	 * Construct a new MobililtyOperationPlugin with the given name.
	 *
	 * @param name The name to give the plugin for identification purposes
	 */
	PortDrayagePlugin(std::string);
	/**
	 * Constructor without paramaters 
	 */
	virtual ~PortDrayagePlugin();
	int Main();
protected:
	/**
	 * Update Configuration
	 */
	void UpdateConfigSettings();

	// Virtual method overrides.
	/**
	 * Method triggers UpdateConfigSettings() on configuration changes
	 */
	void OnConfigChanged(const char *key, const char *value);

	void OnStateChange(IvpPluginState state);
	/**
	 * Method to create port drayage payload JSON ptree using a PortDrayage_Object.
	 * 
	 * @param pd_obj Port Drayage object.
	 * @return json ptree
	 */
	ptree createPortDrayageJson( PortDrayage_Object &pd_obj);
	/**
	 * Method to create MobilityOperation XML ptree.
	 * 
	 * @param ptree json payload
	 * @return MobilityOperation message XML ptree
	 */
	ptree createMobilityOperationXml( ptree &json_payload);

	/**
	 * Handle MobilityOperation message.
	 * 
	 * @param tsm3Message J2735 MobilityOperation message
	 * @param routeableMsg JSON MobilityOperation message
	 */
	void HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg);
	/**
	 * Retrieve next action from freight table using action_id
	 * 
	 * @param action_id string
	 */
	PortDrayage_Object retrieveNextAction( std::string action_id );
	/**
	 * Retrieve first action from first_action table using cmv_id.
	 * 
	 * @param cmv_id 
	 * @return PortDrayage_Object of first action 
	 */
	PortDrayage_Object retrieveFirstAction( std::string cmv_id );

	/**
	 * Create PortDrayage_Object from ptree JSON.
	 * 
	 * @param pr PortDrayage JSON
	 * @return PortDrayage_Object 
	 */
	PortDrayage_Object readPortDrayageJson( ptree &pr );

	/**
	 * Dynamically inserts HOLDING_AREA action into mysql table between
	 * current_action and next_action. Current action should be PORT_CHECKPOINT
	 * and next_action should be EXIT_PORT
	 * 
	 * @param current_action PORT_CHECKPOINT action
	 */
	void insert_holding_action_into_table(PortDrayage_Object &current_action );

	/**
	 * Retrieves HOLDING_AREA action when provided with PORT_CHECKPOINT action
	 * from mysql freight table.
	 * 
	 * @return action_id of HOLDING_AREA action
	 */
	std::string retrieve_holding_inspection_action_id( std::string action_id );

	
private: 
	// Database configuration values
	std::string _database_username;
	std::string _database_password;
	uint16_t _database_port;
	std::string _database_ip;
	std::string _database_name; 
	

	sql::Driver *driver;
	sql::Connection *con;

	// Prepared Statements
	sql::PreparedStatement *next_action_id;
	sql::PreparedStatement *current_action;
	sql::PreparedStatement *first_action;
	sql::PreparedStatement *insert_action;
	sql::PreparedStatement *get_action_id_for_previous_action;
	sql::PreparedStatement *update_current_action;

	// Message Factory for J2735 messages
	J2735MessageFactory factory;
	
	// Web Service Client 
	WebServiceClient *client;

	// Port HOLDING_AREA Configuration
	double _holding_lat;
	double _holding_lon;

};
std::mutex _cfgLock;

}
#endif