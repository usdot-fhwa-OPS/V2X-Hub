//==========================================================================
// Name        : ExamplePlugin.cpp
// Author      : Battelle Memorial Institute
// Version     :
// Copyright   : Copyright (c) 2014 Battelle Memorial Institute. All rights reserved.
// Description : Example Plugin
//==========================================================================

#include "ExamplePlugin.h"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

namespace ExamplePlugin
{

/**
 * Construct a new ExamplePlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
ExamplePlugin::ExamplePlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter<DecodedBsmMessage>(this, &ExamplePlugin::HandleDecodedBsmMessage);

	// This is an internal message type that is used to track some plugin data that changes
	AddMessageFilter<DataChangeMessage>(this, &ExamplePlugin::HandleDataChangeMessage);

	AddMessageFilter<MapDataMessage>(this, &ExamplePlugin::HandleMapDataMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
}

ExamplePlugin::~ExamplePlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void ExamplePlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	GetConfigValue("Instance", instance);

	GetConfigValue("Frequency", __frequency_mon.get());
	__frequency_mon.check();
}


void ExamplePlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void ExamplePlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		SetStatus("ReceivedMaps", 0);
	}
}

void ExamplePlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	static std::atomic<int> count {0};

	PLOG(logINFO) << "New MAP: " << msg;

	int mapCount = count;
	SetStatus("ReceivedMaps", mapCount);
}

void ExamplePlugin::HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg)
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

// Example of handling
void ExamplePlugin::HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logINFO) << "Received a data change message: " << msg;

	PLOG(logINFO) << "Data field " << msg.get_untyped(msg.Name, "?") <<
			" has changed from " << msg.get_untyped(msg.OldValue, "?") <<
			" to " << msg.get_untyped(msg.NewValue, to_string(_frequency));
}


void ExamplePlugin::CreateConnection(){
	try
	{
		// if (ExamplePlugin::_signSimClient != NULL)
		// 	delete ExamplePlugin::_signSimClient;

		// _signSimClient = new UdpClient("192.168.55.49", 6053);
		PLOG(logERROR) << "UDP Client made the connection";

		// char buffer[240];
		// sprintf(buffer, "1.3.6.1.4.1.1206.4.2.1.6.3.1.2.2.1");

		// _signSimClient->Send(buffer, strlen(buffer));






    netsnmp_session session1, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;

    /*
     * Initialize the SNMP library
     */
    init_snmp("snmpdemoapp");

    /*
     * Initialize a "session" that defines who we're going to talk to
     */
    snmp_sess_init( &session1 );                   /* set up defaults */
    session1.peername = strdup("192.168.55.49");

    /* set up the authentication parameters for talking to the server */



    /* set the SNMP version number */
    session1.version = SNMP_VERSION_1;

    /* set the SNMPv1 community name used for authentication */
    session1.community = (u_char*)"public";
    session1.community_len = strlen("public");

    /*
     * Open the session1
     */
    SOCK_STARTUP;
    ss = snmp_open(&session1);                     /* establish the session1 */

    if (!ss) {
      snmp_sess_perror("ack", &session1);
      SOCK_CLEANUP;
      exit(1);
    }
    
    /*
     * Create the PDU for the data for our request.
     *   1) We're going to GET the system.sysDescr.0 node.
     */

	u_long ltmp = 1;

	const char tmp_int[16]= "1";

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;
    if (!snmp_parse_oid("1.3.6.1.4.1.1206.4.2.1.6.3.1.2.2", anOID, &anOID_len)) {
      snmp_perror(".1.3.6.1.4.1.1206.4.2.1.6.3.1.2.2");
      SOCK_CLEANUP;
      exit(1);
    }

    snmp_add_null_var(pdu, anOID, anOID_len);
  
    /*
     * Send the Request out.
     */
    status = snmp_synch_response(ss, pdu, &response);

    /*
     * Process the response.
     */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
      /*
       * SUCCESS: Print the result variables
       */

      for(vars = response->variables; vars; vars = vars->next_variable)
        print_variable(vars->name, vars->name_length, vars);

      /* manipuate the information ourselves */
      for(vars = response->variables; vars; vars = vars->next_variable) {
        if (vars->type == ASN_OCTET_STR) {
	  char *sp = (char *)malloc(1 + vars->val_len);
	  memcpy(sp, vars->val.string, vars->val_len);
	  sp[vars->val_len] = '\0';
          printf("value #%d is a string: %s\n", count++, sp);
	  free(sp);
	}
        else
          printf("value #%d is NOT a string! Ack!\n", count++);
      }
    } else {
      /*
       * FAILURE: print what went wrong!
       */

      if (status == STAT_SUCCESS)
        fprintf(stderr, "Error in packet\nReason: %s\n",
                snmp_errstring(response->errstat));
      else if (status == STAT_TIMEOUT)
        fprintf(stderr, "Timeout: No response from %s.\n",
                session1.peername);
      else
        snmp_sess_perror("snmpdemoapp", ss);

    }

    /*
     * Clean up:
     *  1) free the response.
     *  2) close the session1.
     */
    if (response)
      snmp_free_pdu(response);
    snmp_close(ss);

    SOCK_CLEANUP;

	PLOG(logERROR) << "end end end \n\n\n\n\n";


	}
	catch (UdpClientRuntimeError &ex)
	{
		PLOG(logERROR) << "UDP Client for ExamplePlugin could not be created: " << ex.what();
		HandleException(ex, false);
	}
}

// Override of main method of the plugin that should not return until the plugin exits.
// This method does not need to be overridden if the plugin does not want to use the main thread.
int ExamplePlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{
		PLOG(logDEBUG4) << "Sleeping 1 ms" << endl;

		this_thread::sleep_for(chrono::milliseconds(10));

		msCount += 10;

		// Example showing usage of _frequency configuraton parameter from main thread.
		// Access is thread safe since _frequency is declared using std::atomic.
		if (_plugin->state == IvpPluginState_registered && _frequency <= msCount)
		{
			PLOG(logINFO) << _frequency << " ms wait is complete.";

			PLOG(logINFO) << _frequency << "trying to make a UDP connection.";
			CreateConnection();
			PLOG(logINFO) << _frequency << " UDP connection sucsesfully made.";

			msCount = 0;
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace ExamplePlugin */

int main(int argc, char *argv[])
{
	return run_plugin<ExamplePlugin::ExamplePlugin>("ExamplePlugin", argc, argv);
}
