/**
 * Copyright (C) 2019 LEIDOS.
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
// constant for MobilityOperation strategy	
static CONSTEXPR const char *PORT_DRAYAGE_STRATEGY = "carma/port_drayage";

// Enumeration for different operations
enum Operation {
	PICKUP,DROPOFF,CHECKPOINT,HOLDING,ENTER_STAGING,EXIT_STAGING,ENTER_PORT,EXIT_PORT
};

std::string operation_to_string( Operation operation ) {
	switch(operation) {
		case PICKUP:
			return "PICKUP";
		case DROPOFF:
			return "DROPOFF";
		case CHECKPOINT:
			return "PORT_CHECKPOINT";
		case HOLDING:
			return "HOLDING_AREA";
		case ENTER_STAGING:
			return "ENTER_STAGING_AREA";
		case EXIT_STAGING:
			return "EXIT_STAGING_AREA";
		case ENTER_PORT:
			return "ENTER_PORT";
		case EXIT_PORT:
			return "EXIT_PORT";
		default:
			return "INVALID_OPERATION";
	}
} 
/**
 * PortDrayagePlugin is a V2X-Hub plugin for automating Freight truck interactions for 
 * moving containers between and Port and a Staging Area. The plugin process MobilityOperation
 * messages with the strategy PORT_DRAYAGE_STRATEGY. The payload of this MobilityOperation 
 * message is a JSON object consisting of mainly :
 * 	action_id : unique identifier for the action
 * 	vehicle_id : unique static identifier for the vehicle (NOT BSMID)
 * 	operation : string enumeration describing the action that is to take place
 * This Plugin requires a MySQL database with two table to store all vehicle action (table name: freight) and 
 * to store the first action for any given vehicle (table name : first_action). Upon initial communication
 * with V2X-Hub the vehicle will just send an ENTER_STAGING_AREA actions with no action ID. When this initial
 * message is received by V2X-Hub it will query the first_action table for the vehicle first action. Every action
 * in the database is linked to a next action. Once the vehicle completes an action it notifies V2X-Hub of completion
 * by sending out the action it just completed. V2X-Hub will then query the database for the next linked action
 *    
 * @author Paul Bourelly
 * @version 6.2
 */ 
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
	ptree createPortDrayageJson( const PortDrayage_Object &pd_obj);
	/**
	 * Method to create MobilityOperation XML ptree.
	 * 
	 * @param ptree json payload
	 * @return MobilityOperation message XML ptree
	 */
	ptree createMobilityOperationXml( const ptree &json_payload);

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
	PortDrayage_Object retrieveNextAction( const std::string &action_id );
	/**
	 * Retrieve first action from first_action table using cmv_id.
	 * 
	 * @param cmv_id 
	 * @return PortDrayage_Object of first action 
	 */
	PortDrayage_Object retrieveFirstAction( const std::string &cmv_id );

	/**
	 * Create PortDrayage_Object from ptree JSON.
	 * 
	 * @param pr PortDrayage JSON
	 * @return PortDrayage_Object 
	 */
	PortDrayage_Object readPortDrayageJson( const ptree &pr );

	/**
	 * Dynamically inserts HOLDING_AREA action into mysql table between
	 * current_action and next_action. Current action should be PORT_CHECKPOINT
	 * and next_action should be EXIT_PORT
	 * 
	 * @param current_action PORT_CHECKPOINT action
	 */
	void insert_holding_action_into_table(const PortDrayage_Object &current_action );

	/**
	 * Retrieves HOLDING_AREA action when provided with PORT_CHECKPOINT action
	 * from mysql freight table.
	 * 
	 * @return action_id of HOLDING_AREA action
	 */
	std::string retrieve_holding_inspection_action_id( const std::string &action_id );

	
private: 
	// Database configuration values
 
	

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
	std::shared_ptr<WebServiceClient> client;

	// Port HOLDING_AREA Configuration
	double _holding_lat;
	double _holding_lon;

};
std::mutex _cfgLock;

}
#endif