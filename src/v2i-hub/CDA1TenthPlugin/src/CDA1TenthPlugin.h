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
#include <OAIHelpers.h>
#include <QCoreApplication>
#include "BSMConverter.h"
#include "MobilityOperationConverter.h"
#include "ActionConverter.h"

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
	//TODO NEEDS DESCRIPTION
	void OnStateChange(IvpPluginState state);

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
	/**
	 * Retrieve the next action in the SQL database.
	 * 
	 * @param action_id Action ID of a given start action
	 */
	Action_Object retrieveNextAction(const int &action_id);
	/**
	* @brief Handle BasicSafetyMessage
	* @param msg BsmMessage
	*/
	void receiveBasicSafetyMessage(BsmMessage &msg);


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
	sql::PreparedStatement *prev_action_id;
	sql::PreparedStatement *update_current_action;
	sql::PreparedStatement *action_is_notify;

	// Message Factory for J2735 messages
	J2735MessageFactory factory;
	
	// TODO New Web Service Client 
	// std::shared_ptr<WebServiceClient> client;

};
std::mutex _cfgLock;

}
#endif