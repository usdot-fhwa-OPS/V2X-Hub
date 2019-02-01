/*
 * MessageContext.h
 *
 *  Created on: Jul 29, 2014
 *      Author: ivp
 */

#ifndef MESSAGECONTEXT_H_
#define MESSAGECONTEXT_H_

#include "DbContext.h"
#include <string>
#include <vector>
#include <set>

struct MessageActivityEntry {
	unsigned int id;
	unsigned int messageTypeId;
	unsigned int pluginId;
	unsigned int count;
	time_t lastReceivedTimestamp;
	uint64_t averageInterval;
};

struct MessageTypeEntry {
	unsigned int id;
	std::string type;
	std::string subtype;
	std::string description;

	MessageTypeEntry()
	{
		this->id = 0;
	}

	MessageTypeEntry(std::string type, std::string subtype)
	{
		this->id = 0;
		this->type = type;
		this->subtype = subtype;
	}

	bool operator== (const MessageTypeEntry &cmp) const
	{
		return (this->type + std::string("||") + this->subtype) == (cmp.type + std::string("||") + cmp.subtype);
	}

	bool operator< (const MessageTypeEntry &cmp) const
	{
		return (this->type + std::string("||") + this->subtype) < (cmp.type + std::string("||") + cmp.subtype);
	}
};

class MessageContext : public DbContext
{
public:
	MessageContext();

	void insertOrUpdateMessageActivity(MessageActivityEntry &entry);
	void insertMessageType(MessageTypeEntry &entry, bool updateDescriptionOnDuplicate = false);

	void mapPluginToMessageType(unsigned int pluginId, unsigned int messageTypeId);

	std::set<MessageTypeEntry> getAllMessageTypes();
};


#endif /* MESSAGECONTEXT_H_ */
