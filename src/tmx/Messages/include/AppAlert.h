/*
 * AppAlert.h
 *
 *  Created on: Oct 4, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_APPALERT_H_
#define INCLUDE_APPALERT_H_


#include <string>
#include <ApplicationMessageEnumTypes.h>
#include <tmx/messages/routeable_message.hpp>

/*Wrapper for Application Messages. Convenient for plugins that track multiples.*/
class AppAlert {
public:
	std::string EventId;
	tmx::messages::appmessage::Severity Severity;
	tmx::messages::appmessage::EventCodeTypes EventCode;
	// Timestamp of the original message that triggered this alert. Needed to analyze latency.
	std::string Timestamp;

	AppAlert() {
		EventId = "";
		Severity = tmx::messages::appmessage::Severity::Info;
		EventCode = tmx::messages::appmessage::EventCodeTypes::NOEVENTID;
		tmx::routeable_message timer;
		timer.refresh_timestamp();

		Timestamp = std::to_string(timer.get_timestamp());
	}

	AppAlert(std::string eventId, tmx::messages::appmessage::Severity severity, tmx::messages::appmessage::EventCodeTypes eventCode,
			uint64_t timestamp = 0) {
		EventId = eventId;
		Severity = severity;
		EventCode = eventCode;
		setTimestamp(timestamp);
	}

	AppAlert(tmx::messages::appmessage::Severity severity, tmx::messages::appmessage::EventCodeTypes eventCode,
			uint64_t timestamp = 0) {
		EventId = "";
		Severity = severity;
		EventCode = eventCode;
		setTimestamp(timestamp);
	}



    bool operator ==(const AppAlert& op) const
    {
       if(EventCode == op.EventCode && Severity == op.Severity && EventId == op.EventId)
       {
          return true;
       }
       return false;
    }

    bool operator <(const AppAlert& op) const
    {
       if(Severity < op.Severity)
       {
          return true;
       }
       return false;
    }

    void setTimestamp(uint64_t tStamp)
    {
    	if (tStamp == 0)
    	{
			tmx::routeable_message timer;
			timer.refresh_timestamp();
			tStamp = timer.get_timestamp();
		}
		Timestamp = std::to_string(tStamp);
    }
};





#endif /* INCLUDE_APPALERT_H_ */
