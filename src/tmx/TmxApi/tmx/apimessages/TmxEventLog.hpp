/*
 * @file TmxEventLog.hpp
 *
 *  Created on: Apr 26, 2016
 *      Author: ivp
 */

#ifndef TMX_APIMESSAGES_TMXEVENTLOG_HPP_
#define TMX_APIMESSAGES_TMXEVENTLOG_HPP_

#include <tmx/messages/message.hpp>
#include "IvpEventLog.h"

namespace tmx {
namespace messages {

class TmxEventLogMessage: public tmx::message
{
public:
	TmxEventLogMessage(): message() {}

	TmxEventLogMessage(std::string info): message()
	{
		set_level(IvpLogLevel_info);
		set_description(info);
	}

	TmxEventLogMessage(const std::exception &ex, const std::string &info = "", bool fatal = true): message()
	{
		if (fatal)
			set_level(IvpLogLevel_error);
		else
			set_level(IvpLogLevel_warn);

		std::string descript = info;
		descript += ex.what();
		set_description(descript);
	}

	static constexpr const char *MessageType = IVPMSG_TYPE_APIRESV_EVENTLOG;
	static constexpr const char *MessageSubType = "";

	std_attribute(this->msg, IvpLogLevel, level, IvpLogLevel_info, );
	std_attribute(this->msg, std::string, description, "", );

	IvpEventLogEntry *get_entry()
	{
		entry.level = this->get_level();
		entry.description = const_cast<char *>(this->get_description().c_str());
		return &entry;
	}
private:
	IvpEventLogEntry entry;
};

} /* End namespace messages */
} /* End namespace tmx */

#endif /* TMX_APIMESSAGES_TMXEVENTLOG_HPP_ */
