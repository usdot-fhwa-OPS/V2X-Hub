/*
 * SystemContext.h
 *
 *  Created on: Feb 10, 2016
 *      Author: Svetlana Jacoby
 */

#ifndef SYSTEMCONTEXT_H_
#define SYSTEMCONTEXT_H_

#include <string>
#include <vector>
#include <set>
#include <map>
#include <atomic>
#include <tmx/IvpMessage.h>
#include "DbConnectionPool.h"
#include "../SystemContextThread.h"
//#include "../PluginClient.h"

#define SYS_CONTEXT_CONFIGKEY_DB_RFRSH_INTERVAL "MsgLatencyDbRefreshInterval"

using namespace std;
using namespace tmx;

namespace tmx {
namespace utils {


struct MessageTypeEntry {
	std::string msgType;
	std::string msgSubtype;

	MessageTypeEntry()
	{
	}

	MessageTypeEntry(std::string type, std::string subtype)
	{
		this->msgType = type;
		this->msgSubtype = subtype;
	}

	bool operator== (const MessageTypeEntry &cmp) const
	{
		return (this->msgType + std::string("||") + this->msgSubtype) == (cmp.msgType + std::string("||") + cmp.msgSubtype);
	}

	bool operator< (const MessageTypeEntry &cmp) const
	{
		return (this->msgType + std::string("||") + this->msgSubtype) < (cmp.msgType + std::string("||") + cmp.msgSubtype);
	}
};

struct MessageLatencyEntry {
	string sPluginName; // Sender
	MessageTypeEntry msgEntry;
	uint64_t msgCreatedTimestamp;		// in milliseconds
	uint64_t msgHandledTimestamp;		// in milliseconds
	// Timestamp of the initial message that triggered this sequence of events.
	uint64_t origMsgTimestamp;		    // in milliseconds

	MessageLatencyEntry()
	{
		this->sPluginName = "";
		this->msgCreatedTimestamp = 0;
		this->msgHandledTimestamp = 0;
		this->origMsgTimestamp = 0;
	}

	MessageLatencyEntry(string plugin, std::string type, std::string subtype, uint64_t cTime, uint64_t hTime)
	{
		this->sPluginName = plugin;
		this->msgEntry.msgType = type;
		this->msgEntry.msgSubtype = subtype;
		this->msgCreatedTimestamp = cTime;
		this->msgHandledTimestamp = hTime;
		this->origMsgTimestamp = cTime;
	}
};

struct ReceivedPluginEntry {
	uint64_t msgReceivedTimestamp;		// in MICROSECONDS
	string rPluginName; // Receiver

	ReceivedPluginEntry(uint64_t rTime, string plugin)
	{
		this->msgReceivedTimestamp = rTime;
		this->rPluginName = plugin;
	}

	bool operator== (const ReceivedPluginEntry &cmp) const
	{
		return (to_string(this->msgReceivedTimestamp) + std::string("||") + this->rPluginName) == (to_string(cmp.msgReceivedTimestamp) + std::string("||") + cmp.rPluginName);
	}

	bool operator< (const ReceivedPluginEntry &cmp) const
	{
		return (to_string(this->msgReceivedTimestamp) + std::string("||") + this->rPluginName) < (to_string(cmp.msgReceivedTimestamp) + std::string("||") + cmp.rPluginName);
	}
};

class SystemContext
{
	friend class SystemContextThread;

public:
	SystemContext();
	virtual ~SystemContext();

	void trackMessageHandled(string pName, IvpMessage *msg, uint64_t receivedTimeMicroSec);
	void setDbUpdateFrequency(const char *val);
	static void updateLatencyDb();

protected:
	static std::map<MessageTypeEntry, uint32_t> getAllMessageTypes(tmx::utils::DbConnection &conn);

private:
	static map <ReceivedPluginEntry, MessageLatencyEntry> _msgLatencyMap;

	std::atomic<bool> trackingEnabled{false};

	mutex dbThreadLock;
	static mutex latencyMapLock;
	mutex dbUpdateLock;

    // Separate DB updater thread is created to update DB with latency values.
	SystemContextThread * _sysContextDbUpdater{nullptr};
};

}}

#endif /* SYSTEMCONTEXT_H_ */
