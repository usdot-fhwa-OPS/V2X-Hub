//============================================================================
// Name        : ivpcore.cpp
// Author      : Battelle Memorial Institute
// Version     :
// Copyright   : Copyright (c) 2014 Battelle Memorial Institute. All rights reserved.
// Description : Launch the Core of the IVP System
//============================================================================

#include "MessageRouterBasic.h"
#include "PluginServer.h"
#include "PluginConnection.h"
#include "PluginMonitor.h"
#include "MessageProfiler.h"
#include "logger.h"
#include "HistoryManager.h"

#include "database/PluginContext.h"
#include "database/ConfigContext.h"
#include "database/MessageContext.h"

#include "tmx/tmx.h"
#include "tmx/messages/IvpJ2735.h"

#include <signal.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string.h>


#include <boost/process.hpp>

#define CONFIGKEY_LOG_FILE_NAME "LOG_FILE_NAME"

sighandler_t oldsig_int;
sighandler_t oldsig_kill;
sighandler_t oldsig_quit;
sighandler_t oldsig_term;
sighandler_t oldsig_segv;

void sig(int sig)
{
	std::stringstream logDescription;
	LogLevel logLevel = LogLevel_Info;

	logDescription << "IVP Core shutting down: Received ";
	switch(sig)
	{
	case SIGINT:
		logDescription << "SIGINT";
		break;
	case SIGKILL:
		logDescription << "SIGKILL";
		break;
	case SIGQUIT:
		logDescription << "SIGQUIT";
		break;
	case SIGTERM:
		logDescription << "SIGTERM";
		break;
	case SIGSEGV:
		logDescription << "SIGSEGV";
		logLevel = LogLevel_Fatal;
		break;
	default:
		logDescription << "unknown signal";
		break;
	}

	dhlogging::Logger::addEventLogEntry(LOG_SOURCE_CORE, logDescription.str(), logLevel);

	try {
		PluginContext pcontext;

		pcontext.setStatusForAllPlugins(IVP_STATUS_STOPPED_DISCONENCTED);
		pcontext.removeAllPluginStatusItems();
	} catch (DbException &e) {

	}

	signal(SIGINT, oldsig_int);
	signal(SIGKILL, oldsig_kill);
	signal(SIGQUIT, oldsig_quit);
	signal(SIGTERM, oldsig_term);
	signal(SIGSEGV, oldsig_segv);
	raise(sig);
}

void addSystemDefinedMessageTypes()
{
	//TODO handle this some other way so that the core isn't including anything from the api?
	IvpMessageTypeCollection *entries = NULL;
	for(int id = 0; id <= 255; id++)//id <= DSRCmsgID_travelerInformation; id++)
	{
		entries = ivpJ2735_addMsgTypeToCollection(entries, (e_DSRCmsgID)id);
	}

	std::vector<MessageTypeEntry> dbEntries;

	//TODO HACK this is all in PluginConnection
	int arraySize = ivpMsgType_getEntryCount(entries);
	for(int i = 0; i < arraySize; i++)
	{
		IvpMessageTypeEntry *entry = ivpMsgType_getEntry(entries, i);
		assert(entry != NULL);

		MessageTypeEntry dbEntry(std::string(entry->type), std::string(entry->subtype));
		dbEntry.description = entry->description ? std::string(entry->description) : "";
		dbEntries.push_back(dbEntry);

		ivpMsgType_destroyEntry(entry);
	}

	try {
		MessageContext mcontext;
		for(std::vector<MessageTypeEntry>::iterator itr = dbEntries.begin(); itr != dbEntries.end(); itr++)
		{
			mcontext.insertMessageType(*itr, true);
		}
	} catch (const DbException &e) {
		LOG_WARN(e.what());
	}

	if (entries != NULL)
		ivpMsgType_destroyCollection(entries);

} 

char* getRootPsw() {
	char* psw;
	psw = std::getenv("MYSQL_ROOT_PASSWORD");

	if(psw == NULL){
		LOG_ERROR("Unable to set MYSQL_ROOT_PASSWORD)");
		return "";
	}
	else{
		return psw;
	}
}

int main()
{
	//std::string env_p(std::getenv("MYSQL_ROOT_PASSWORD"));
	std::string env_p(getRootPsw());

	DbContext::ConnectionInformation.url = "127.0.0.1";
	DbContext::ConnectionInformation.username = "IVP";
	DbContext::ConnectionInformation.password = env_p;
	DbContext::ConnectionInformation.db = "IVP";

	oldsig_int = signal(SIGINT, sig);
	oldsig_kill = signal(SIGKILL, sig);
	oldsig_quit = signal(SIGQUIT, sig);
	oldsig_term = signal(SIGTERM, sig);
	oldsig_segv = signal(SIGSEGV, sig);

	SystemConfigurationParameterEntry logFileName = SystemConfigurationParameterEntry(CONFIGKEY_LOG_FILE_NAME, "ivpcore.log");

	try {
		ConfigContext ccontext;
		ccontext.initializeSystemConfigParameter(&logFileName);
	} catch (DbException &e) {
		dhlogging::Logger::getInstance(logFileName.value);
		LOG_ERROR("Unable to initialize core configuration values [" << e.what() << "]");
	}

	dhlogging::Logger::getInstance(logFileName.value);

	/*
	 * !!!!!!!!!!!!!!!!!!!!!!!! Any initialization code goes below this line !!!!!!!!!!!!!!!!!!!!!!!!
	 */

	dhlogging::Logger::addEventLogEntry("--------", "------------------------", LogLevel_Debug);
	dhlogging::Logger::addEventLogEntry(LOG_SOURCE_CORE, "IVP Core Starting", LogLevel_Info);
	sleep(1);

	try {
		PluginContext pcontext;
		pcontext.setStatusForAllPlugins(IVP_STATUS_STOPPED_DISCONENCTED);
	} catch (DbException &e) {
		LOG_WARN("Unable to initialize all plugin statuses to " << IVP_STATUS_STOPPED_DISCONENCTED << " [" << e.what() << "]");
	}

	addSystemDefinedMessageTypes();

	MessageRouterBasic messageRouter;
	PluginServer pluginServer(&messageRouter);
	PluginMonitor pluginMonitor(&messageRouter);
	MessageProfiler messageProfiler(&messageRouter);
	HistoryManager historyManager(&messageRouter);

	while(1) {
		sleep(10);
	}

	return 0;
}

