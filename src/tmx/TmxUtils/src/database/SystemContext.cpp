/*
 * SystemContext.cpp
 *
 *  Created on: Feb 10, 2016
 *      Author: Svetlana Jacoby
 */

#include <assert.h>
#include <sstream>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include <tmx/messages/routeable_message.hpp>
#include <ApplicationMessage.h>
#include "../PluginLog.h"
#include "../Clock.h"
#include "SystemContext.h"

using namespace std;
using namespace std::chrono;
using namespace tmx::messages;

#define INSERT_PLUGIN_ACTIVTY_STMT \
	"INSERT INTO IVP.pluginActivity ( \
		`msgReceivedTimestamp`, \
		`rPluginName`, \
		`sPluginName`, \
		`msgType`, \
		`msgSubtype`, \
		`msgCreatedTimestamp`, \
		`msgHandledTimestamp`, \
		`origMsgTimestamp` \
	 ) VALUES ( \
	    ?, ?, ?, ?, ?, ?, ?, ? \
     );"

namespace tmx {
namespace utils {

map <ReceivedPluginEntry, MessageLatencyEntry> SystemContext::_msgLatencyMap;
mutex SystemContext::latencyMapLock;

SystemContext::SystemContext()
{
	FILE_LOG(logDEBUG) << "Constructing static SystemContext";

	// Start a thread that will periodically flush latency entries to the DB.
	static ThreadTimer updateTimer(milliseconds(100)); // this is precision in ms. Should be a small value to allow frequency setting to be high.

	SystemContextThread * dbT = new SystemContextThread(updateTimer);
	{
		lock_guard<mutex> lock(dbThreadLock);
		_sysContextDbUpdater = dbT;
		_sysContextDbUpdater->set_Frequency(chrono::milliseconds(0)); // default to not log

	}
	// Start the pvd timer.  No signals will be sent until valid configuration is received.
	updateTimer.Start();
}

SystemContext::~SystemContext()
{
	lock_guard<mutex> lock(dbThreadLock);
	if (_sysContextDbUpdater)
		delete _sysContextDbUpdater;
}

void SystemContext::trackMessageHandled(string pName, IvpMessage *msg, uint64_t receivedTimeMicroSec)
{
	if (trackingEnabled)
	{
		routeable_message routeableMsg(msg);

		// An easy way to keep timing is to use refresh timestamp on a message
		static routeable_message timer;
		timer.refresh_timestamp();

		ReceivedPluginEntry key(receivedTimeMicroSec, pName);
		MessageLatencyEntry value(routeableMsg.get_source(),routeableMsg.get_type(),
				routeableMsg.get_subtype(), routeableMsg.get_timestamp(), timer.get_timestamp());

		if ( routeableMsg.get_type() == ApplicationMessage::MessageType	&&
				routeableMsg.get_subtype() == ApplicationMessage::MessageSubType )
		{
			ApplicationMessage applicationMessage;

			applicationMessage.set_contents(routeableMsg.get_payload_str());
			value.origMsgTimestamp = stoul(applicationMessage.get_Timestamp());
		}
		else
		{
			value.origMsgTimestamp = routeableMsg.get_timestamp();
		}
		lock_guard<mutex> lock(latencyMapLock);
		_msgLatencyMap[key] = value;
	}
}


void SystemContext::setDbUpdateFrequency(const char *val)
{
	if (stoi(val) == 0)
	{
		trackingEnabled = false;
		lock_guard<mutex> lock(latencyMapLock);
		_msgLatencyMap.clear();
	}
	else
	{
		trackingEnabled = true;
	}

	lock_guard<mutex> lock(dbThreadLock);
	_sysContextDbUpdater->set_Frequency(chrono::milliseconds(stoi(val)));
}

void SystemContext::updateLatencyDb()
{
	FILE_LOG(logDEBUG) << "Updating latency Db...";
	FILE_LOG(logDEBUG) << "_msgLatencyMap size is: " << _msgLatencyMap.size();
	map <ReceivedPluginEntry, MessageLatencyEntry> copy;
	{
		lock_guard<mutex> lock(latencyMapLock);
		_msgLatencyMap.swap(copy);
	}

	static tmx::utils::DbConnectionPool dbConnPool;
	tmx::utils::DbConnection conn = dbConnPool.Connection();

	std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(INSERT_PLUGIN_ACTIVTY_STMT));

	uint32_t numInserts = 0;
	for (std::map<ReceivedPluginEntry, MessageLatencyEntry>::iterator it=copy.begin(); it!=copy.end(); ++it)
	{
		pstmt->setUInt64(1, it->first.msgReceivedTimestamp);
		pstmt->setString(2, it->first.rPluginName);
		pstmt->setString(3, it->second.sPluginName);
		pstmt->setString(4, it->second.msgEntry.msgType);
		pstmt->setString(5, it->second.msgEntry.msgSubtype);
		pstmt->setUInt64(6, it->second.msgCreatedTimestamp);
		pstmt->setUInt64(7, it->second.msgHandledTimestamp);
		pstmt->setUInt64(8, it->second.origMsgTimestamp);

		numInserts += pstmt->executeUpdate();
	}
	FILE_LOG(logDEBUG) << numInserts << " insterted into latency Db!";
}

}}

