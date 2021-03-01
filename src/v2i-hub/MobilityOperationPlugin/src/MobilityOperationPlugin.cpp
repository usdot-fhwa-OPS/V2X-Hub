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
	
	FILELog::ReportingLevel() = FILELog::FromString("INFO");
	AddMessageFilter < tsm3Message > (this, &MobilityOperationPlugin::HandleMobilityOperationMessage);
	SubscribeToMessages();

}

MobilityOperationPlugin::~MobilityOperationPlugin() {
}

void MobilityOperationPlugin::UpdateConfigSettings() {

}

void MobilityOperationPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void MobilityOperationPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg ) {
	auto mobilityOperation = msg.get_j2735_data();
	// FILE_LOG(logDEBUG) << "Checking log level : " << FILELog::ReportingLevel();
	PLOG(logDEBUG) << "Received MobilityOperation message (encoded) : " << routeableMsg.get_payload_str();
	PLOG(logDEBUG) << "Header Host BSM ID : " << mobilityOperation->header.hostBSMId.buf;
	PLOG(logDEBUG) << "Header Host Static ID : " << mobilityOperation->header.hostStaticId.buf;
	PLOG(logDEBUG) << "Header Plan ID : " << mobilityOperation->header.planId.buf;
	PLOG(logDEBUG) << "Header Target Static ID : " << mobilityOperation->header.targetStaticId.buf;
	PLOG(logDEBUG) << "Header Timestamp : " << mobilityOperation->header.timestamp.buf;
	PLOG(logDEBUG) << "Body OperationParams : " << mobilityOperation->body.operationParams.buf;
	PLOG(logDEBUG) << "Body Strategy : " << mobilityOperation->body.strategy.buf;


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

