/*
 * RtcmMessage.h
 *
 * This file contains the base RTCM message for use in TMX.
 * There are significant differences in the content by version and
 * by message type.  Therefore, a template specialization is declared
 * elsewhere that details the specific contents for that message version
 * and type.  The classes here only provide a generic implementations
 * across each RTCM version and message type.
 *
 *  Created on: May 11, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCMMESSAGE_H_
#define INCLUDE_RTCM_RTCMMESSAGE_H_

#include <atomic>
#include <bitset>
#include <tuple>

#include <tmx/messages/message.hpp>
#include <tmx/j2735_messages/RtcmMessage.hpp>

#include "RtcmTypes.h"
#include "RtcmVersion.h"


// The default TMX message namespace is tmx::messages
namespace tmx {
namespace messages {
namespace rtcm {

typedef uint16_t msgtype_type;


} /* End namespace rtcm */

/**
 * The virtual base class for all RTCM messages
 */
class TmxRtcmMessage: public tmx::message {
public:
	TmxRtcmMessage(): tmx::message() { }
	TmxRtcmMessage(const tmx::message_container_type &contents):
		tmx::message(contents) { }
	TmxRtcmMessage(const TmxRtcmMessage &other):
		tmx::message(other) { }
	virtual ~TmxRtcmMessage() { clear(); }

	static constexpr const char *MessageType = tmx::messages::RtcmMessage::MessageSubType;
	static constexpr const char *MessageSubType = "Unknown";

	// Virtual functions.
	virtual rtcm::RTCM_VERSION get_Version() { return rtcm::RTCM_VERSION::UNKNOWN; };
	virtual rtcm::msgtype_type get_MessageType() { return 0; };
	virtual size_t size() { return _bytes.size(); }
	virtual void set_contents(const tmx::byte_stream &bytes) { _bytes = bytes; }
	virtual tmx::byte_stream get_contents() { return _bytes; }
	virtual bool is_Valid() { return _valid; }
	virtual tmx::messages::RtcmMessage get_RtcmMessage() { static tmx::messages::RtcmMessage blank; return blank; }

	// Pre-defined functions
	std::string get_VersionName() { return rtcm::RtcmVersionName(get_Version()); }

	using tmx::message::set_contents;
	void clear() { _bytes.clear(); tmx::message::clear(); }
protected:
	void invalidate() { _valid = false; }
	tmx::byte_stream &getBytes() { return _bytes; }
private:
	bool _valid = true;
	tmx::byte_stream _bytes;
};

/**
 *  The base template instance for all RTCM message implementation by version.
 */
template <rtcm::RTCM_VERSION Version = rtcm::UNKNOWN>
class RTCMMessage: public TmxRtcmMessage {
public:
	RTCMMessage(): TmxRtcmMessage() { }
	RTCMMessage(const TmxRtcmMessage &other): TmxRtcmMessage(other) { }
	virtual ~RTCMMessage() { }

	virtual rtcm::RTCM_VERSION get_Version() { return Version; }
};

/**
 * The base template instance for all RTCM message type implementation by version and type
 */
template <rtcm::RTCM_VERSION Version = rtcm::UNKNOWN, rtcm::msgtype_type Type = 0>
class RTCMMessageType: public RTCMMessage<Version> {
public:
	RTCMMessageType(): RTCMMessage<Version>() { }
	RTCMMessageType(const TmxRtcmMessage &other): RTCMMessage<Version>(other) { }
	virtual ~RTCMMessageType() { }

	virtual rtcm::RTCM_VERSION get_Version() { return Version; }
	virtual rtcm::msgtype_type get_MessageType() { return Type; }
};

namespace rtcm {

template <RTCM_VERSION Version>
struct RtcmMessageTypeBox {
	typedef std::tuple<RTCMMessageType<Version, 0> > types;
};

} /* End namespace rtcm */
} /* End namespace messages */
} /* End namespace tmx */


#ifndef IGNORE_RTCM2
#include "RTCM2.h"
#endif
#ifndef IGNORE_RTCM3
#include "RTCM3.h"
#endif

#include "RtcmEncodedMessage.h"
#endif /* INCLUDE_RTCM_RTCMMESSAGE_H_ */
