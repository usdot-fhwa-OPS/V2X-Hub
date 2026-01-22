/*
 * routeable_message.hpp
 *
 *  Created on: Apr 7, 2016
 *      Author: ivp
 */

#ifndef TMX_MESSAGES_ROUTEABLE_MESSAGE_HPP_
#define TMX_MESSAGES_ROUTEABLE_MESSAGE_HPP_

#include <sys/time.h>
#include <tmx/IvpMessage.h>
#include <tmx/tmx.h>
#include <tmx/attributes/attribute_wrapped_type.hpp>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/messages/message.hpp>

#define ATTR_HEADER "header"
#define DSRC_HEADER "dsrcMetadata"
#define ATTR_PAYLOAD "payload"
#define UNKNOWN_TYPE "Unknown"
#define UNKNOWN_SUBTYPE "Unknown"

namespace tmx
{

/**
 * A class to represent a specific type of message with attributes that are used to route through
 * the TMX core.  There is a header portion of the message which contains attributes that can be used
 * to identify the type of the message and the source, and a payload portion that contains the actual message
 * data.  Most message types are just other tmx::message types in the same format, but can also be a string
 * containing the data.  The type and subtype attributes in the header are used to determine information about
 * the payload, and the encoding attribute may be used to determine how the data is encoded.  Typically, this will
 * be using the Format encoding, whose default is JSON.  Other Formats and other encodings must be serialized as
 * a string payload.
 */
template <typename Format = TMX_DEFAULT_MESSAGE_FORMAT>
class tmx_routeable_message: public tmx_message<Format>
{
public:
	/**
	 * Create an empty message, or optionally from an incoming IVP message
	 */
	tmx_routeable_message(IvpMessage *other = 0):
		tmx_message<Format>() { set_contents(other); }

	/**
	 * Create a message with a copy of the message container contents.  Nothing is initialized.
	 * @param contents The already filled message container
	 */
	tmx_routeable_message(const message_container_type &contents):
		tmx_message<Format>(contents) { reinit(); }

	/**
	 * Create a message that has a copy of the contents of the other message.
	 * @see tmx::message(const message<OtherFormat> &, message_converter *)
	 * @param other The message to copy from
	 * @param converter A specific converter to use, or NULL for the default one
	 */
	template <typename OtherFormat>
	tmx_routeable_message(const tmx_routeable_message<OtherFormat> &other, message_converter *converter = 0):
		tmx_message<Format>(other, converter) { set_contents(other.ivpMsg); }

	/**
	 * Create a message that has a copy of the contents of the other message.
	 * @see tmx::message(const message<OtherFormat> &, message_converter *)
	 * @param other The message to copy from
	 */
	tmx_routeable_message(const tmx_routeable_message<Format> &other):
		tmx_routeable_message<Format>(other, 0) { set_contents(other.get_message()); }

	/**
	 * Create a message from a string representation of the contents.
	 * @see tmx::message(const std::string &)
	 * @param contents A string representation of the contents.
	 */
	tmx_routeable_message(const std::string &contents):
		tmx_message<Format>(contents) { reinit(); }

	/**
	 * Destructor
	 */
	virtual ~tmx_routeable_message() { destroy(); }

private:
	template <typename OtherFormat>
	void fill(tmx_message<OtherFormat> *msg)
	{
		// If the payload is a sub-tree, set the contents to a copy of that tree.
		// Otherwise, use the string value of the payload
		boost::optional<message_tree_type &> payload = this->as_tree(ATTR_PAYLOAD);
		if (payload)
			msg->set_contents(payload.get());
		else
			msg->set_contents(get_payload_str());
	}

	template <typename OtherFormat>
	void fill(tmx_routeable_message<OtherFormat> *rMsg)
	{
		// Some older messages are actually routeable messages in themselves.  If that is the case, then
		// this needs to return just a copy of the whole message in that new type
		rMsg->set_contents(ivpMsg);
	}
public:
	// Some methods used internally must be explicitly called in from base class
	using tmx::tmx_message<Format>::set_contents;
	using tmx::tmx_message<Format>::clear;
	using tmx::tmx_message<Format>::is_empty;
	using tmx::tmx_message<Format>::to_string;


	/**
	 * Get the payload as the specified message type.  Note that the message type class must be of
	 * type tmx::message.  If the payload is a string, then the contents of the returned message are
	 * set using tmx::message::set_contents(string).  Otherwise, the contents are set using
	 * tmx::message::set_contents(message_tree_type &) with the sub-tree.
	 * @return A new message of the specified type constructed with the payload contents
	 */
	template <typename MessageType>
	MessageType get_payload()
	{
		MessageType newMsg;
		fill(&newMsg);
		return newMsg;
	}

	/**
	 * @return A string representation of the payload
	 */
	std::string get_payload_str()
	{
		std::string payloadStr;

		// The payload object is most recent
		if (!ivpMsg->payload)
			return payloadStr;

		// Convert payload to a string
		char *json = cJSON_PrintUnformatted(ivpMsg->payload);
		if (json) payloadStr.assign(json);

		// The cJSON print code may add quotation marks.
		if (payloadStr.length() > 0 && payloadStr[0] == '\"')
			payloadStr.erase(0, 1);
		if (payloadStr.length() > 0 && payloadStr[payloadStr.length()-1] == '\"')
			payloadStr.erase(payloadStr.length()-1);
		free(json);

		if (ivpMsg->payload->type == cJSON_Object || ivpMsg->payload->type == cJSON_Array)
		{
			message_container_type container;
			std::stringstream ss;
			ss << payloadStr;
			container.load<JSON>(ss);
			this->as_tree().get().put_child(ATTR_PAYLOAD, container.get_storage().get_tree());
		}
		else
		{
			this->msg.store(ATTR_PAYLOAD, payloadStr);
		}

		return payloadStr;
	}

	/**
	 * @see get_payload_str()
	 * @return A byte stream representation of the payload string
	 */
	byte_stream get_payload_bytes()
	{
		return battelle::attributes::attribute_lexical_cast<byte_stream>(this->get_payload_str());
	}

	/**
	 * Set the payload with the given message.  Note that the payload should be in the same format.
	 * @param payload The message payload
	 */
	void set_payload(tmx_message<Format> &payload)
	{
		message_tree_type copy(this->as_tree(payload.get_container()).get());
		this->as_tree().get().put_child(ATTR_PAYLOAD, copy);

		// Set the encoding based on the format.  In lowercase.
		std::string format = this->format();
		std::transform(format.begin(), format.end(), format.begin(), ::tolower);
		this->set_encoding(format);

		// For JSON encoding, parse the payload from a JSON string
		if (strncmp("json", get_encoding().c_str(), 4) == 0)
		{
			message_container_type container(payload.get_container());
			std::stringstream ss;
			container.template save<JSON>(ss);
			this->ivpMsg->payload = cJSON_Parse(ss.str().c_str());
		}
	}

	/**
	 * Set the payload with the given message of another format.  Note that a string representation
	 * of the contents will be used and the encoding type will be "<format>string", such as
	 * "jsonstring" or "xmlstring".
	 * @param payload The message payload
	 */
	template <typename OtherFormat>
	void set_payload(tmx_message<OtherFormat> &payload)
	{
		this->set_payload(payload.to_string());

		// Set the encoding based on the format.  In lowercase.
		std::string format = payload.format();
		std::transform(format.begin(), format.end(), format.begin(), ::tolower);
		format.append(IVP_ENCODING_STRING);

		this->set_encoding(format);
	}

	/**
	 * Set the payload with the given string.  Note that the encoding type will be "string".
	 * @param payload The payload string
	 */
	void set_payload(std::string payload)
	{
		this->set_encoding(IVP_ENCODING_STRING);
		this->msg.store(ATTR_PAYLOAD, payload);
		this->ivpMsg->payload = cJSON_CreateString(payload.c_str());
	}

	/**
	 * Set the payload with the given bool.  Note that the encoding type will be "string".
	 */
	void set_payload(bool payload)
	{
		this->set_encoding(IVP_ENCODING_STRING);
		this->msg.store(ATTR_PAYLOAD, payload ? "1" : "0");
		this->ivpMsg->payload = cJSON_CreateBool(payload ? 1 : 0);
	}

	/**
	 * Set the payload with the given number.  Note that the encoding type will be "string".
	 */
	void set_payload(int number)
	{
		this->set_encoding(IVP_ENCODING_STRING);
		this->msg.store(ATTR_PAYLOAD, to_string(number));
		this->ivpMsg->payload = cJSON_CreateNumber((double)number);
	}

	/**
	 * Set the payload with the given number.  Note that the encoding type will be "string".
	 */
	void set_payload(double number)
	{
		this->set_encoding(IVP_ENCODING_STRING);
		this->msg.store(ATTR_PAYLOAD, to_string(number));
		this->ivpMsg->payload = cJSON_CreateNumber(number);
	}

	/**
	 * Set the payload with the given byte stream.  Note that the encoding type will be "bytearray/hexstring"
	 * @param bytes The payload bytes
	 */
	void set_payload_bytes(const byte_stream &bytes)
	{
		set_payload(battelle::attributes::attribute_lexical_cast<std::string>(bytes));
		this->set_encoding(IVP_ENCODING_BYTEARRAY);
	}

	/**
	 * Set the payload with the given IVP message C-structure.  This can be done one of two ways, either by
	 * holding on to a copy of this source message, and ultimately serializing in a lazy way, when needed.
	 * This is the default behavior, and should perform better by eliminating extra serialization steps when
	 * processing the message.
	 */
	void set_contents(const IvpMessage *msg)
	{
		destroy();

		if (msg)
		{
			ivpMsg = ivpMsg_copy(const_cast<IvpMessage *>(msg));
		}
		else
		{
			ivpMsg = ivpMsg_create(NULL, NULL, NULL, 0, NULL);
		}

		// Bind the header attributes
		_type.bind(ivpMsg->type);
		_subtype.bind(ivpMsg->subtype);
		_source.bind(ivpMsg->source);
		_sourceId.bind(ivpMsg->sourceId);
		_encoding.bind(ivpMsg->encoding);
		_timestamp.bind(ivpMsg->timestamp);
		_flags.bind(ivpMsg->flags);

		// Note that the DSRC metadata is optional, so only bind when set
		if (ivpMsg->dsrcMetadata)
		{
			_dsrcChannel.bind(ivpMsg->dsrcMetadata->channel);
			_dsrcPsid.bind(ivpMsg->dsrcMetadata->psid);
		}

		init_attributes();
	}

	/**
	 * Re-initialize the message from the container.  This is typically only done when the container is updated
	 * outside of the message contents, which then need to be reloaded.
	 */
	void reinit()
	{
		if (is_empty())
		{
			// We cannot re-initialize on an empty container.
			// Just flush out the current IVP message
			std::string msgContents(ivpMsg_createJsonString(ivpMsg, IvpMsg_FormatOptions_none));
			set_contents(msgContents);
		}

		// Reload the pointer from the contents
		set_contents(ivpMsg_parse(const_cast<char*>(to_string().c_str())));
	}

	/**
	 * Initialize the message contents, given a payload message
	 * @param payload The message payload
	 * @param source The source of the message
	 * @param sourceId The source ID of the message
	 * @param flags Flags for the TMX router
	 */
	template <typename MessageType>
	void initialize(MessageType &payload, const std::string source = "", unsigned int sourceId = 0, unsigned int flags = 0)
	{
		this->initialize(MessageType::MessageType, MessageType::MessageSubType, source, sourceId, flags);
		this->set_payload(payload);
	}

	/**
	 * Initialize the message contents, given only the payload type and subtype
	 * @param messageType The payload message type
		return ivpMsg;

	 * @param messageSubType The payload message subtype
	 * @param source The source of the message
	 * @param sourceId The source ID of the message
	 * @param flags Flags for the TMX router
	 */
	virtual void initialize(std::string messageType, std::string messageSubType,
			const std::string source = "", unsigned int sourceId = 0, unsigned int flags = 0)
	{
		this->set_type(messageType);
		this->set_subtype(messageSubType);
		this->refresh_timestamp();
		if (source.length() > 0) this->set_source(source);
		if (sourceId > 0) this->set_sourceId(sourceId);
		if (flags > 0) this->set_flags(flags);
	}
private:
	// Header attributes

    struct IvpMessageHeaderCreator
	{
		typedef message_path_type path_type;

		path_type operator()(path_type attr_path)
		{
			path_type p(ATTR_HEADER);
			std::string tmp(attr_path.dump());
			if (attr_path.dump().compare(0, 4, "dsrc") == 0)
			{
				// DSRC attributes go under a separate sub-object
				p /= DSRC_HEADER;
				tmp.replace(0, 4, "");
				tmp[0] = tolower(tmp[0]);
				p /= tmp;
			}
			else
			{
				p /= attr_path;
			}
			return p;
		}
	};

    template <typename T>
    using ivp_header_attribute_base =
    		battelle::attributes::standard_2level_attribute<T, message_container_type::storage_type, IvpMessageHeaderCreator>;

    template <typename T>
    using ivp_header_attribute =
    		battelle::attributes::standard_wrapped_attribute<ivp_header_attribute_base<T> >;

    template <typename T>
    using ivp_str_header_attribute =
    		battelle::attributes::standard_wrapped_attribute<ivp_header_attribute_base<T>, battelle::attributes::StringAllocator>;

 	rw_attribute(this->msg, ivp_str_header_attribute<type>, std::string, type, get_, "Unknown", set_, )
    rw_attribute(this->msg, ivp_str_header_attribute<subtype>, std::string, subtype, get_, "Unknown", set_, )
	rw_attribute(this->msg, ivp_str_header_attribute<source>, std::string, source, get_, "", set_, )
	rw_attribute(this->msg, ivp_header_attribute<sourceId>, unsigned int, sourceId, get_, 0, set_, )
	rw_attribute(this->msg, ivp_str_header_attribute<encoding>, std::string, encoding, get_, "", set_, )
	rw_attribute(this->msg, ivp_header_attribute<timestamp>, uint64_t, timestamp, get_, 0, set_, )
	rw_attribute(this->msg, ivp_header_attribute<flags>, unsigned int, flags, get_, 0, set_, )
	rw_attribute(this->msg, ivp_header_attribute<dsrcChannel>, int, dsrcChannel, get_, -1, set_, )
	rw_attribute(this->msg, ivp_header_attribute<dsrcPsid>, int, dsrcPsid, get_, -1, set_, )
	rw_attribute(this->msg, ivp_str_header_attribute<dsrcRsuIp>, std::string, dsrcRsuIp, get_, "", set_, )
	rw_attribute(this->msg, ivp_header_attribute<dsrcRsuPort>, int, dsrcRsuPort, get_, 0, set_, )

public:
	// Some other helpful routines

	/**
	 * Retrieve a copy of the current message pointer.  The user is responsible for memory cleanup.
	 *
	 * @return The underlying IVP message
	 */
	IvpMessage* get_message()
	{
		return ivpMsg_copy(ivpMsg);
	}

	/**
	 * Retrieve a const version of the current message pointer.  Note that <b>no copy</b> is made, and
	 * no changes can be made to the underlying structure.  Therefore, use the non-const version of
	 * this function in order to obtain a mutable version.
	 *
	 * @return The underlying IVP message
	 * @see get_message()
	 */
	IvpMessage* get_message() const
	{
		return ivpMsg;
	}

	static uint64_t get_millisecondsSinceEpoch()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		double millisecondsSinceEpoch =
		    (double)(tv.tv_sec) * 1000 +
		    (double)(tv.tv_usec) / 1000;

		return millisecondsSinceEpoch;
	}

	/**
	 * Reset the timestamp attribute to the current time
	 */
	void refresh_timestamp()
	{
		this->set_timestamp(get_millisecondsSinceEpoch());
	}

	/**
	 * Add DSRC metadata attributes
	 * @param channel The V2X channel, default set to 183. Configurable in Immediate Forward Plugin.
	 * @param psid The V2X psid
	 */
	void addDsrcMetadata(int psid, int channel = 183)
	{
		ivpMsg_addDsrcMetadata(ivpMsg, psid, channel);
		_dsrcChannel.bind(ivpMsg->dsrcMetadata->channel);
		_dsrcPsid.bind(ivpMsg->dsrcMetadata->psid);
	}

	virtual void flush() {
		get_payload_str();
	}

private:
	// For incoming messages, keep a copy of the source IVP message
	IvpMessage *ivpMsg = NULL;

	void destroy()
	{
		if (ivpMsg)
			ivpMsg_destroy(ivpMsg);
	}

	void init_attributes()
	{
		// Initialize type, sub-type, encoding, timestamp and flags in that order to ensure they get set
		this->get_type();
		this->get_subtype();
		this->get_encoding();
		this->get_timestamp();
		this->get_flags();
	}
};

/// A TMX routeable message in JSON format
typedef tmx_routeable_message<JSON> json_routeable_message;
/// A TMX routeable message in XML format
typedef tmx_routeable_message<XML> xml_routeable_message;
/// A TMX routeable message in the default format
typedef tmx_routeable_message<TMX_DEFAULT_MESSAGE_FORMAT> routeable_message;

} /* End namespace tmx */

#endif /* TMX_MESSAGES_ROUTEABLE_MESSAGE_HPP_ */
