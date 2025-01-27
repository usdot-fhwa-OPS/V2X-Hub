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
#ifndef SRC_CDA1TenthPlugin_H_
#define SRC_CDA1TenthPlugin_H_
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
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <OAIDefaultApi.h>
#include <QEventLoop>
#include <QTimer>
#include <thread>
#include <OAIHelpers.h>
#include <QCoreApplication>
#include "BSMConverter.h"
#include "WebSocketServer.h"

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace boost::property_tree;
using namespace OpenAPI;

namespace CDA1TenthPlugin {
// constant for MobilityOperation strategy	
static CONSTEXPR const char *PORT_DRAYAGE_STRATEGY = "carma/port_drayage";
// TODO future implementation
// static CONSTEXPR const char *RIDESHARE_STRATEGY = "rideshare";

// Enumeration for different operations
// !! Rework needed to handle more generic operation, remove switch case
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
 * TODO DESCRIPTION
 *    
 * @author Peyton Johnson
 * @version 6.2
 */ 
class CDA1TenthPlugin: public PluginClient {
public:
	struct CDA1Tenth_Object {
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
	CDA1TenthPlugin(std::string);
	/**
	 * Constructor without paramaters 
	 */
	virtual ~CDA1TenthPlugin();
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
	 * Method to create port drayage payload JSON ptree using a CDA1Tenth_Object.
	 * 
	 * @param cda1t_obj Port Drayage object.
	 * @return json ptree
	 */
	ptree createCDA1TenthJson( const CDA1Tenth_Object &cda1t_obj);
	/**
	 * Create CDA1Tenth_Object from ptree JSON.
	 * 
	 * @param pr CDA1Tenth JSON
	 * @return CDA1Tenth_Object 
	 */
	CDA1Tenth_Object readCDA1TenthJson( const ptree &pr );

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
	 * Handle BasicSafety message.
	 * 
	 * @param BsmMessage J2735 BasicSafety message
	 * @param routeableMsg JSON BasicSafety message
	 */
	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);
	CDA1Tenth_Object retrieveNextAction(const std::string &action_id);
	CDA1Tenth_Object retrieveFirstAction(const std::string &cmv_id);
	/**
	* @brief Handle BasicSafetyMessage
	* @param msg BsmMessage
	*/
	void receiveBasicSafetyMessage(BsmMessage &msg);
	void startWebsocketServer();
private: 
	// Database configuration values
	sql::Driver *driver;
	sql::Connection *con;
	std::string Key_BSMMessageSkipped = "bsmMessageSkipped";
	int _bsmMessageSkipped = 0;
	
	// Prepared Statements
	sql::PreparedStatement *next_action_id;
	sql::PreparedStatement *current_action;
	sql::PreparedStatement *first_action;
	sql::PreparedStatement *insert_action;
	sql::PreparedStatement *get_action_id_for_previous_action;
	sql::PreparedStatement *update_current_action;

	// Message Factory for J2735 messages
	J2735MessageFactory factory;
	
	// TODO New Web Service Client 
	// std::shared_ptr<WebServiceClient> client;

	// Port HOLDING_AREA Configuration
	double _holding_lat;
	double _holding_lon;

	std::shared_ptr<WebSocketServer> ws;
	uint16_t bsm_forward_frequency = 1;
	uint64_t bsm_last_forward_time_ms = 0;

};
std::mutex _cfgLock;

}
#endif