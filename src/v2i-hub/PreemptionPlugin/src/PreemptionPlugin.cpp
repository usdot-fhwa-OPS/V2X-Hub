//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include "PreemptionPlugin.hpp"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

namespace PreemptionPlugin
{

/**
 * Construct a new PreemptionPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
PreemptionPlugin::PreemptionPlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter<DecodedBsmMessage>(this, &PreemptionPlugin::HandleDecodedBsmMessage);

	// This is an internal message type that is used to track some plugin data that changes
	AddMessageFilter<DataChangeMessage>(this, &PreemptionPlugin::HandleDataChangeMessage);

	AddMessageFilter<MapDataMessage>(this, &PreemptionPlugin::HandleMapDataMessage);

	AddMessageFilter <BsmMessage> (this, &PreemptionPlugin::HandleBasicSafetyMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
}

PreemptionPlugin::~PreemptionPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void PreemptionPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	GetConfigValue("Instance", instance);
	GetConfigValue("Frequency", __frequency_mon.get());
	__frequency_mon.check();

	GetConfigValue("BasePreemptionOid", BasePreemptionOid);
	GetConfigValue("ipwithport", ipwithport);
	GetConfigValue("snmp_community", snmp_community);
	GetConfigValue("map_path", map_path);

}


void PreemptionPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void PreemptionPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		SetStatus("ReceivedMaps", 0);
	}
}

void PreemptionPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	static std::atomic<int> count {0};

	PLOG(logINFO) << "New MAP: " << msg;

	int mapCount = count;
	SetStatus("ReceivedMaps", mapCount);
}

void PreemptionPlugin::HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg)
{
	//PLOG(logDEBUG) << "Received Decoded BSM: " << msg;

	// Determine if location, speed, and heading are valid.
	bool isValid = msg.get_IsLocationValid() && msg.get_IsSpeedValid() && msg.get_IsHeadingValid();

	// Print some of the BSM values.
	PLOG(logDEBUG) << "ID: " << msg.get_TemporaryId()
		<< ", Location: (" <<  msg.get_Latitude() << ", " <<  msg.get_Longitude() << ")"
		<< ", Speed: " << msg.get_Speed_mph() << " mph"
		<< ", Heading: " << msg.get_Heading() << "°"
		<< ", All Valid: " << isValid
		<< ", IsOutgoing: " << msg.get_IsOutgoing();
}

void PreemptionPlugin::HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logINFO) << "Received a data change message: " << msg;

	PLOG(logINFO) << "Data field " << msg.get_untyped(msg.Name, "?") <<
			" has changed from " << msg.get_untyped(msg.OldValue, "?") <<
			" to " << msg.get_untyped(msg.NewValue, to_string(_frequency));
}

void PreemptionPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) {

	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";

	mp->VehicleLocatorWorker(&msg);

	// if(preemption_plan_flag == "1" && mp->preemption_plan_flag.c_str() == "0") {
	// 	preemption_plan_flag = mp->preemption_plan_flag.c_str();
	// 	std::string PreemptionOid = BasePreemptionOid + preemption_plan;
	// 	int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag);
	// 	if(response != 0){
	// 		PLOG(logINFO) << "Sending oid intrupted with an error.";
	// 	}
	// 	else{
	// 		PLOG(logINFO) << "Finished sending preemption plan.";
	// 	}
	// }
	// else if(preemption_plan_flag != "1"){ 
	// 	preemption_plan = mp->preemption_plan;
	// 	preemption_plan_flag = mp->preemption_plan_flag.c_str();
	// 	std::string PreemptionOid = BasePreemptionOid + preemption_plan;
	// 	int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag);
	// 	if(response != 0){
	// 		PLOG(logINFO) << "Sending oid intrupted with an error.";
	// 	}
	// 	else{
	// 		PLOG(logINFO) << "Finished sending preemption plan.";
	// 	}
	// }

	// PLOG(logINFO) << "preemption_plan = " << preemption_plan << " preemption_plan_flag = " << preemption_plan_flag << std::endl << std::endl;

}

void PreemptionPlugin::GetInt32(unsigned char *buf, int32_t *value) {
	*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
}

int PreemptionPlugin::SendOid(const char *PreemptionOid, const char *value) {

	netsnmp_session session, *ss;
	netsnmp_pdu    *pdu, *response = NULL;
	netsnmp_variable_list *vars;
	oid             name[MAX_OID_LEN];
	size_t          name_length;
	int             status;
	int             failures = 0;
	int             exitval = 0;

	init_snmp("snmpset");
	snmp_sess_init(&session);
	session.peername = strdup(ipwithport.c_str());
	session.version = snmp_version;
	session.community = (u_char *)snmp_community.c_str();
	session.community_len = strlen((const char*) session.community);
	session.timeout = 1000000;

	SOCK_STARTUP;

	ss = snmp_open(&session);

	if (ss == NULL) {
		snmp_sess_perror("snmpset", &session);
		SOCK_CLEANUP;
		exit(1);
	}

	// create PDU for SET request and add object names and values to request 
	
	pdu = snmp_pdu_create(SNMP_MSG_SET);
	
	name_length = MAX_OID_LEN;
	if (snmp_parse_oid(PreemptionOid, name, &name_length) == NULL) {
		snmp_perror(PreemptionOid);
		failures++;
	} else {
		if (snmp_add_var
			(pdu, name, name_length, 'i', value)) {
			snmp_perror(PreemptionOid);
			failures++;
		}
	}

	if (failures) {
		snmp_close(ss);
		SOCK_CLEANUP;
		exit(1);
	}

	//send the request 
	
	status = snmp_synch_response(ss, pdu, &response);
	if (status == STAT_SUCCESS) {
		if (response->errstat == SNMP_ERR_NOERROR) {
			if (1) {
				print_variable(response->variables->name, response->variables->name_length, response->variables);
			}
		} else {
			fprintf(stderr, "Error in packet.\nReason: %s\n", snmp_errstring(response->errstat));
			exitval = 2;
		}
	} else if (status == STAT_TIMEOUT) {
		fprintf(stderr, "Timeout: No Response from %s\n", session.peername);
		exitval = 1;
	} else {                    /* status == STAT_ERROR */
		snmp_sess_perror("snmpset", ss);
		exitval = 1;
	}

	if (response)
		snmp_free_pdu(response);
	snmp_close(ss);
	SOCK_CLEANUP;

	return exitval;
}

int PreemptionPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{
		if(mp->map == nullptr){
			PLOG(logINFO) << "loading map ... " << std::endl;
			mp->ProcessMapMessageFile(map_path);
		}

		mp->ip_with_port = ipwithport;
		mp->snmp_version = SNMP_VERSION_1;
		mp->snmp_community = snmp_community;
		mp->base_preemption_oid = BasePreemptionOid;

		PLOG(logDEBUG4) << "Sleeping 1 ms" << endl;

		this_thread::sleep_for(chrono::milliseconds(10));

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered && _frequency <= msCount)
		{
			PLOG(logINFO) << _frequency << " ms wait is complete.";

			this_thread::sleep_for(chrono::milliseconds(1000));

			msCount = 0;
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace PreemptionPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PreemptionPlugin::PreemptionPlugin>("PreemptionPlugin", argc, argv);
}
