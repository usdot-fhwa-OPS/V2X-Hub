/*
 * RtcmEncodedMessage.h
 *
 *  Created on: May 11, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCMENCODEDMESSAGE_H_
#define INCLUDE_RTCM_RTCMENCODEDMESSAGE_H_

#include <memory>
#include <tmx/TmxException.hpp>
#include <tmx/messages/routeable_message.hpp>

#include "RtcmMessage.h"
#include "RtcmMessageFactory.h"

namespace tmx {
namespace messages {

class TmxRtcmEncodedMessage: public tmx::routeable_message {
	typedef std::vector<TmxRtcmMessage *> rtcm_container;

public:
	TmxRtcmEncodedMessage(): tmx::routeable_message() {
		tmx::routeable_message::initialize(blank);
	}
	TmxRtcmEncodedMessage(const tmx::routeable_message &other):
		tmx::routeable_message(other) {
		this->reinit();
	}
	virtual ~TmxRtcmEncodedMessage() { clear_container(); }

	size_t size() {
		return get_container().size();
	}

	typename rtcm_container::iterator begin() {
		return get_container().begin();
	}

	typename rtcm_container::iterator end() {
		return get_container().end();
	}

	template <rtcm::RTCM_VERSION Version>
	void set_rtcm_message(RTCMMessage<Version> &msg) {
		set_rtcm_message(&msg);
	}

	template <rtcm::RTCM_VERSION Version, rtcm::msgtype_type Type>
	void set_rtcm_message(RTCMMessageType<Version, Type> &msg) {
		set_rtcm_message(&msg);
	}

	template <rtcm::RTCM_VERSION Version>
	void initialize(RTCMMessage<Version> &msg, const std::string source = "", unsigned int sourceId = 0, unsigned int flags = 0) {
		tmx::routeable_message::initialize(RTCMMessage<Version>::MessageType, msg.get_VersionName(), source, sourceId, flags);
		set_rtcm_message(msg);
	}

	template <rtcm::RTCM_VERSION Version, rtcm::msgtype_type Type>
	void initialize(RTCMMessageType<Version, Type> &msg, const std::string source = "", unsigned int sourceId = 0, unsigned int flags = 0) {
		tmx::routeable_message::initialize(RTCMMessageType<Version, Type>::MessageType, msg.get_VersionName(), source, sourceId, flags);
		set_rtcm_message(msg);
	}

	void initialize(TmxRtcmMessage &msg, const std::string source = "", unsigned int sourceId = 0, unsigned int flags = 0) {
		tmx::routeable_message::initialize(TmxRtcmMessage::MessageType, msg.get_VersionName(), source, sourceId, flags);
		set_rtcm_message(&msg);
	}

private:
	TmxRtcmMessage blank;

	rtcm_container _myMessages;

	rtcm_container &get_container() {
		static rtcm::RtcmMessageFactory factory;
		rtcm::RTCM_VERSION thisVer = rtcm::RtcmVersion(this->get_subtype());

		if (_myMessages.size() <= 0) {
			// Fill the container
			tmx::byte_stream copy(this->get_payload_bytes());

			while (copy.size() > 0) {
				TmxRtcmMessage *ptr = NULL;

				rtcm::RTCM_VERSION v = (thisVer ? thisVer : (rtcm::RTCM_VERSION)((int)rtcm::RTCM_EOF - 1));
				do {
					ptr = factory.create(v);

					if (ptr)
						ptr->set_contents(copy);

					v = (rtcm::RTCM_VERSION)((int)v - 1);
				} while (!(ptr && ptr->is_Valid()) && v > thisVer);

				if (ptr->is_Valid()) {
					// Help with decoding in the future by setting the known version
					this->set_subtype(ptr->get_VersionName());
					thisVer = ptr->get_Version();

					// Reconstruct the message as specific to the type
					if (ptr->get_MessageType()) {
						auto tmp = factory.create(thisVer, ptr->get_MessageType());
						if (tmp) {
							tmp->set_contents(ptr->get_contents());

							if (tmp->is_Valid()) {
								delete ptr;
								ptr = tmp;
							}
						}
					}

					_myMessages.push_back(ptr);
				} else {
					delete ptr;
					ptr = NULL;
				}

				size_t n = copy.size();
				if (ptr && ptr->size() <= copy.size()) n = ptr->size();

				copy.erase(copy.begin(), copy.begin() + n);
			}
		}

		return _myMessages;
	}

	void clear_container() {
		for (size_t i = 0; i < _myMessages.size(); i++) {
			delete _myMessages[i];
			_myMessages[i] = NULL;
		}

		_myMessages.clear();
	}

	void set_rtcm_message(TmxRtcmMessage *msg) {
		clear_container();
		if (msg) this->set_payload_bytes(msg->get_contents());
		else this->set_payload("");
	}

};


} /* End namespace messages */
} /* End namespace tmx */



#endif /* INCLUDE_RTCM_RTCMENCODEDMESSAGE_H_ */
