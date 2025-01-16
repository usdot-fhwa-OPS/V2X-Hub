/**
 * Copyright (C) 2024 LEIDOS.
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

namespace CDA1TenthPlugin
{

	CDA1TenthPlugin::CDA1TenthPlugin(string name) : PluginClient(name)
	{
		// Plugin Handles MobilityOperation Messages
		AddMessageFilter<tsm3Message>(this, &CDA1TenthPlugin::HandleMobilityOperationMessage);
		AddMessageFilter<BsmMessage>(this, &CDA1TenthPlugin::HandleBasicSafetyMessage);
		SubscribeToMessages();
	}

	CDA1TenthPlugin::~CDA1TenthPlugin()
	{
	}

	void CDA1TenthPlugin::UpdateConfigSettings()
	{

		lock_guard<mutex> lock(_cfgLock);

		std::string _database_username;
		std::string _database_password;
		uint16_t _database_port;
		std::string _database_ip;
		std::string _database_name;
		// Database configuration
		GetConfigValue<string>("Database_Username", _database_username);
		GetConfigValue<string>("Database_Password", _database_password);
		GetConfigValue<string>("Database_IP", _database_ip);
		GetConfigValue<uint16_t>("Database_Port", _database_port);
		GetConfigValue<string>("Database_Name", _database_name);
		GetConfigValue<string>("BSM_Transmit_Topic", _transmitBSMTopic);

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

		// client = std::make_shared<WebServiceClient>(host, port, secure, polling_frequency);
		// Port Holding Area Configurable location
		GetConfigValue<double>("Holding_Lat", _holding_lat);
		GetConfigValue<double>("Holding_Lon", _holding_lon);
		PLOG(logDEBUG) << "Holding Area set : (" << _holding_lat << ", " << _holding_lon << ")" << std::endl;

		// Create DB connection
		std::string connection_string = "tcp://" + _database_ip + ":" + std::to_string(_database_port);
		try
		{
			driver = get_driver_instance();
			con = driver->connect(connection_string, _database_username, _database_password);
			con->setSchema(_database_name);
			// Initialize PreparedStatements for MySQL
			// Get next_action for given action_id
			next_action_id = con->prepareStatement("SELECT next_action FROM freight WHERE action_id = ? ");
			// Get current action for given action_id
			current_action = con->prepareStatement("SELECT * FROM freight WHERE action_id = ? ");
			// Get first action for vehicle
			first_action = con->prepareStatement("SELECT * FROM first_action WHERE cmv_id = ? ");
			// Insert action into freight table
			insert_action = con->prepareStatement("INSERT INTO freight VALUES(?,?,?,?,?, UUID(), ?)");
			// Get action_id of the previous action given action_id
			get_action_id_for_previous_action = con->prepareStatement("SELECT action_id FROM freight WHERE next_action = ? and operation = ? ");
			// Update next_action for an action with given action_id
			update_current_action = con->prepareStatement("UPDATE freight SET next_action = ? WHERE action_id = ?");
		}
		catch (const sql::SQLException &e)
		{
			PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl
										 << e.what() << std::endl
										 << "Error occurred in file " << __FILE__ << " on line " << __LINE__ << std::endl
										 << "Error code " << e.getErrorCode() << std::endl
										 << "Error status " << e.getSQLState() << std::endl;
		}
	}

	void CDA1TenthPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClient::OnConfigChanged(key, value);
		UpdateConfigSettings();
	}

	void CDA1TenthPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg)
	{

		// Retrieve J2735 Message
		auto mobilityOperation = msg.get_j2735_data();

		// String Stream for strategy and operationParams
		std::stringstream strat;
		std::stringstream payload;

		// Create ptree object for json
		ptree pr;

		// Create new CDA1Tenth_Object pointer
		std::unique_ptr<CDA1Tenth_Object> pd(new CDA1Tenth_Object());

		// Read strategy and operationParams
		strat << mobilityOperation->body.strategy.buf;
		payload << mobilityOperation->body.operationParams.buf;

		// Compare strategy to CDA1Tenth strategy
		std::string strategy = strat.str();
		if (strategy.compare(PORT_DRAYAGE_STRATEGY) == 0)
		{
			try
			{
				PLOG(logINFO) << "Body OperationParams : " << mobilityOperation->body.operationParams.buf
											<< std::endl
											<< "Body Strategy : " << mobilityOperation->body.strategy.buf;
				// Convert JSON payload to CDA1Tenth_Object
				read_json(payload, pr);
				*pd = readCDA1TenthJson(pr);
			}
			catch (const ptree_error &e)
			{
				PLOG(logERROR) << "Error parsing Mobility Operation payload: " << e.what() << std::endl;
			}
			// Handle actions that require CDA1Tenth WebService Input
			if (pd->operation.compare(operation_to_string(Operation::PICKUP)) == 0)
			{
				// client->request_loading_action(pd->cmv_id, pd->cargo_id, pd->action_id);
			}
			else if (pd->operation.compare(operation_to_string(Operation::DROPOFF)) == 0)
			{
				// client->request_unloading_action(pd->cmv_id, pd->cargo_id, pd->action_id);
			}
			else if (pd->operation.compare(operation_to_string(Operation::CHECKPOINT)) == 0)
			{
				// If holding == 1 insert HOLDING action into table
				// int holding = client->request_inspection(pd->cmv_id, pd->cargo_id, pd->action_id);
				// if (holding == 1)
				// {
				// 	insert_holding_action_into_table(*pd);
				// }
			}
			else if (pd->operation.compare(operation_to_string(Operation::HOLDING)) == 0)
			{
				// string previous_checkpoint_id = retrieve_holding_inspection_action_id(pd->action_id);
				// client->request_holding(previous_checkpoint_id);
			}

			PLOG(logDEBUG) << "Port Drayage Message" << std::endl
										 << "cmv_id : " << pd->cmv_id << std::endl
										 << "cargo_id : " << pd->cargo_id << std::endl
										 << "cargo : " << pd->cargo << std::endl
										 << "operation : " << pd->operation << std::endl
										 << "location_lat : " << pd->location_lat << std::endl
										 << "location_long : " << pd->location_long << std::endl
										 << "destination_lat : " << pd->destination_lat << std::endl
										 << "destination_long : " << pd->destination_long << std::endl
										 << "action_id : " << pd->action_id << std::endl
										 << "next_action : " << pd->next_action << std::endl;

			try
			{
				// Retrieve first or next action
				CDA1Tenth_Object *new_action = new CDA1Tenth_Object();
				if (pd->action_id.empty())
				{
					PLOG(logDEBUG) << "Retrieving first action." << std::endl;
					*new_action = retrieveFirstAction(pd->cmv_id);
				}
				else
				{
					PLOG(logDEBUG) << "Retrieving next action." << std::endl;
					*new_action = retrieveNextAction(pd->action_id);
				}

				if (!new_action->action_id.empty())
				{
					// Initializer vars
					tsm3Message mob_msg;
					tsm3EncodedMessage mobilityENC;
					tmx::message_container_type container;
					std::unique_ptr<tsm3EncodedMessage> msg;

					//  Create operationParams payload json
					ptree payload = createCDA1TenthJson(*new_action);

					// Create XML MobilityOperationMessage
					ptree message = createMobilityOperationXml(payload);
					std::stringstream content;
					write_xml(content, message);
					try
					{
						// Uper encode message
						container.load<XML>(content);
						mob_msg.set_contents(container.get_storage().get_tree());
						mobilityENC.encode_j2735_message(mob_msg);
						msg.reset();
						msg.reset(dynamic_cast<tsm3EncodedMessage *>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE03_STRING)));
						string enc = mobilityENC.get_encoding();
						PLOG(logDEBUG) << "Encoded outgoing message : " << std::endl
													 << mobilityENC.get_payload_str();
						msg->refresh_timestamp();
						msg->set_payload(mobilityENC.get_payload_str());
						msg->set_encoding(enc);
						msg->set_flags(IvpMsgFlags_RouteDSRC);
						msg->addDsrcMetadata(0xBFEE);
						msg->refresh_timestamp();
						routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
						BroadcastMessage(*rMsg);
					}
					catch (const J2735Exception &e)
					{
						PLOG(logERROR) << "Error occurred during message encoding " << std::endl
													 << e.what() << std::endl;
					}
				}
				else
				{
					PLOG(logWARNING) << "Could not find action!" << std::endl;
				}
			}
			catch (const sql::SQLException &e)
			{
				PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl
											 << e.what() << std::endl
											 << "Error code " << e.getErrorCode() << std::endl
											 << "Error status " << e.getSQLState() << std::endl;
			}
		}
	}

	ptree CDA1TenthPlugin::createMobilityOperationXml(const ptree &json_payload)
	{
		ptree mobilityOperationXml;
		std::stringstream pl;
		write_json(pl, json_payload);
		// Create XML MobilityOperationMessage
		ptree message;
		ptree header;
		ptree body;
		body.put("strategy", PORT_DRAYAGE_STRATEGY);
		body.put("operationParams", pl.str());
		header.put("hostStaticId", "UNSET");
		header.put("targetStaticId", "UNSET");
		header.put("hostBSMId", "00000000");
		header.put("planId", "00000000-0000-0000-0000-000000000000");
		header.put("timestamp", "0000000000000000000");
		message.put_child("header", header);
		message.put_child("body", body);
		mobilityOperationXml.put_child("TestMessage03", message);
		return mobilityOperationXml;
	}

	CDA1TenthPlugin::CDA1Tenth_Object CDA1TenthPlugin::retrieveFirstAction(const std::string &cmv_id)
	{
		std::unique_ptr<CDA1Tenth_Object> rtn(new CDA1Tenth_Object());
		try
		{
			// Set cmv_id
			first_action->setString(1, cmv_id);
			// Retrieve first action of first_action table
			sql::ResultSet *cur_action = first_action->executeQuery();
			if (cur_action->first())
			{
				// Create Port Drayage object
				rtn->cmv_id = cur_action->getString("cmv_id");
				rtn->operation = cur_action->getString("operation");
				rtn->action_id = cur_action->getString("action_id");
				rtn->cargo_id = cur_action->getString("cargo_id");
				rtn->destination_long = cur_action->getDouble("destination_long");
				rtn->destination_lat = cur_action->getDouble("destination_lat");
				rtn->next_action = cur_action->getString("next_action");
				PLOG(logDEBUG) << "Port Drayage Message" << std::endl
											 << "cmv_id : " << rtn->cmv_id << std::endl
											 << "cargo_id : " << rtn->cargo_id << std::endl
											 << "operation : " << rtn->operation << std::endl
											 << "destination_lat : " << rtn->destination_lat << std::endl
											 << "destination_long : " << rtn->destination_long << std::endl
											 << "action_id : " << rtn->action_id << std::endl
											 << "next_action : " << rtn->next_action << std::endl;
				return *rtn;
			}
			else
			{
				PLOG(logERROR) << "No first action for cmv_id : " << cmv_id << " found!";
				return *rtn;
			}
		}
		catch (const sql::SQLException &e)
		{
			PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl
										 << e.what() << std::endl
										 << "Error code " << e.getErrorCode() << std::endl
										 << "Error status " << e.getSQLState() << std::endl;
			return *rtn;
		}
	}

	CDA1TenthPlugin::CDA1Tenth_Object CDA1TenthPlugin::retrieveNextAction(const std::string &action_id)
	{
		std::unique_ptr<CDA1Tenth_Object> rtn(new CDA1Tenth_Object());
		try
		{
			// Set action_id
			next_action_id->setString(1, action_id);
			// Get current action
			sql::ResultSet *cur_action = next_action_id->executeQuery();
			if (cur_action->first())
			{
				std::string next = cur_action->getString("next_action");
				// Set action_id to next_action
				current_action->setString(1, next);
				// Retrieve next action
				sql::ResultSet *cur_action = current_action->executeQuery();
				// Create CDA1Tenth_Object
				if (cur_action->first())
				{
					rtn->cmv_id = cur_action->getString("cmv_id");
					rtn->operation = cur_action->getString("operation");
					rtn->action_id = cur_action->getString("action_id");
					rtn->cargo_id = cur_action->getString("cargo_id");
					rtn->destination_long = cur_action->getDouble("destination_long");
					rtn->destination_lat = cur_action->getDouble("destination_lat");
					rtn->next_action = cur_action->getString("next_action");

					PLOG(logDEBUG) << "Port Drayage Message : " << std::endl
												 << "cmv_id : " << rtn->cmv_id << std::endl
												 << "cargo_id : " << rtn->cargo_id << std::endl
												 << "operation : " << rtn->operation << std::endl
												 << "destination_lat : " << rtn->destination_lat << std::endl
												 << "destination_long : " << rtn->destination_long << std::endl
												 << "action_id : " << rtn->action_id << std::endl
												 << "next_action : " << rtn->next_action << std::endl;
				}
				else
				{
					// If we are able to retrieve the current action but not it's next_action we can
					// assume it was the last action in the DB.
					PLOG(logINFO) << "Last action completed! No action found with action id " << next << std::endl;
				}
			}
		}
		catch (const sql::SQLException &e)
		{
			PLOG(logERROR) << "Error occurred during MYSQL Connection " << std::endl
										 << e.what() << std::endl
										 << "Error code " << e.getErrorCode() << std::endl
										 << "Error status " << e.getSQLState() << std::endl;
		}
		return *rtn;
	}

	ptree CDA1TenthPlugin::createCDA1TenthJson(const CDA1Tenth_Object &cda1t_obj)
	{
		ptree json_payload;
		json_payload.put("cmv_id", cda1t_obj.cmv_id);
		json_payload.put("cargo_id", cda1t_obj.cargo_id);
		ptree destination;
		destination.put<double>("latitude", cda1t_obj.destination_lat);
		destination.put<double>("longitude", cda1t_obj.destination_long);
		json_payload.put_child("destination", destination);
		json_payload.put("operation", cda1t_obj.operation);
		json_payload.put("action_id", cda1t_obj.action_id);
		return json_payload;
	}

	CDA1TenthPlugin::CDA1Tenth_Object CDA1TenthPlugin::readCDA1TenthJson(const ptree &pr)
	{
		std::unique_ptr<CDA1Tenth_Object> pd(new CDA1Tenth_Object());
		try
		{
			pd->cmv_id = pr.get_child("cmv_id").get_value<std::string>();
			boost::optional<const ptree &> child = pr.get_child_optional("action_id");
			if (!child)
			{
				PLOG(logINFO) << "No action_id present! This is the vehicle's first action" << std::endl;
			}
			else
			{
				pd->action_id = child.get_ptr()->get_value<string>();
				// For eventually tracking of completed actions
				pd->operation = pr.get_child("operation").get_value<string>();
				child = pr.get_child_optional("cargo_id");
				if (child)
				{
					pd->cargo_id = pr.get_child("cargo_id").get_value<string>();
				}
				child = pr.get_child_optional("location");
				if (child)
				{
					ptree location = pr.get_child("location");
					pd->location_lat = location.get_child("latitude").get_value<double>();
					pd->location_long = location.get_child("longitude").get_value<double>();
				}
			}
			return *pd.get();
		}
		catch (const ptree_error &e)
		{
			PLOG(logERROR) << "Error parsing Mobility Operation payload: " << e.what() << std::endl;
			return *pd.get();
		}
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
			SetStatus<uint>(Key_BSMMessageSkipped.c_str(), ++_bsmMessageSkipped);
		}
	}

	/**
	 * Trigger when Plugin state changes
	 *
	 * @param state IvpPluginState
	 */
	void CDA1TenthPlugin::OnStateChange(IvpPluginState state)
	{
		PluginClient::OnStateChange(state);

		if (state == IvpPluginState_registered)
		{
			UpdateConfigSettings();
		}
	}

	int CDA1TenthPlugin::Main()
	{
		uint64_t lastSendTime = 0;
		while (_plugin->state != IvpPluginState_error)
		{
			usleep(100000); // sleep for microseconds set from config.
		}
		return (EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[])
{
	// Qt HttpClient setup
	QCoreApplication a(argc, argv);
	return run_plugin<CDA1TenthPlugin::CDA1TenthPlugin>("CDA1TenthPlugin", argc, argv);
}
