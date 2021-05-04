/*
 * @file J2735MessageFactory.hpp
 *
 *  Created on: May 17, 2016
 *      @author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_J2735MESSAGEFACTORY_HPP_
#define TMX_J2735_MESSAGES_J2735MESSAGEFACTORY_HPP_

#include <iomanip>
#include <map>
#include <memory>

#include <tmx/apimessages/TmxEventLog.hpp>
#include <tmx/messages/TmxJ2735.hpp>

// Need to include all the types
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#if SAEJ2735_SPEC < 63
#include <tmx/j2735_messages/BasicSafetyMessageVerbose.hpp>
#endif
#include <tmx/j2735_messages/CommonSafetyRequestMessage.hpp>
#include <tmx/j2735_messages/EmergencyVehicleAlertMessage.hpp>
#include <tmx/j2735_messages/IntersectionCollisionMessage.hpp>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/j2735_messages/NmeaMessage.hpp>
#if SAEJ2735_SPEC < 63
#include <tmx/j2735_messages/PersonalMobilityMessage.hpp>
#endif
#include <tmx/j2735_messages/PersonalSafetyMessage.hpp>
#include <tmx/j2735_messages/ProbeDataManagementMessage.hpp>
#include <tmx/j2735_messages/ProbeVehicleDataMessage.hpp>
#include <tmx/j2735_messages/RoadSideAlertMessage.hpp>
#include <tmx/j2735_messages/RtcmMessage.hpp>
#include <tmx/j2735_messages/SignalRequestMessage.hpp>
#include <tmx/j2735_messages/SignalStatusMessage.hpp>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>
#include <tmx/j2735_messages/testMessage04.hpp>
#include <tmx/j2735_messages/testMessage05.hpp>

namespace tmx {
namespace messages {

struct ENDOFLIST {};

/// Variadic template for the message type list
template <class... T> struct message_type_list {};
using message_types = message_type_list<
		BsmMessage,
#if SAEJ2735_SPEC < 63
		BsmvMessage,
#endif
		CsrMessage,
		EvaMessage,
		IntersectionCollisionMessage,
		MapDataMessage,
		NmeaMessage,
		PdmMessage,
#if SAEJ2735_SPEC < 63
		PmmMessage,
#endif
		PsmMessage,
		PvdMessage,
		RsaMessage,
		RtcmMessage,
		SrmMessage,
		SsmMessage,
		SpatMessage,
		TimMessage,
		tsm4Message,
		tsm5Message
>;

/// Base allocator type
struct message_allocator
{
	virtual ~message_allocator() {}

	/// Get the J2735 message ID for the message
	virtual int getMessageID() { return 0; };
	/// Get the message type name for the message
	virtual const char *getMessageType() { return ""; };
	/// Create a new instance of the J2735 message
	virtual TmxJ2735EncodedMessageBase *allocate() { return NULL; };
};

/// Templated allocator implementation for the messages
template <typename MessageType>
struct message_allocator_impl: public message_allocator
{
	typedef MessageType type;

	virtual ~message_allocator_impl() {}
	/// @see TmxJ2735Message<>::get_default_msgId()
	inline int getMessageID() { return type::get_default_messageId(); }
	/// @see TmxJ2735Message<>::MessageSubType
	inline const char *getMessageType() { return type::MessageSubType; }
	/// @see TmxJ2735Message<>()
	inline TmxJ2735EncodedMessageBase *allocate() { return new TmxJ2735EncodedMessage<type>(); }
};

/**
 * A factory class to create J2735 messages.  The trouble with the various messages is that the programmer must
 * know specifics about what type the message is in order to correctly send the message.  Therefore, this class
 * encompasses all the complexity of creating a new J2735 message type from either the type name or the message
 * ID.
 */
class J2735MessageFactory
{
private:
	using int_map = std::map<int, message_allocator *>;
	using str_map = std::map<std::string, message_allocator *>;

	int_map &byInt;
	str_map &byStr;

	/**
	 * @return The only static instance of the map by message ID
	 */
	static int_map &get_int_map()
	{
		static int_map theMap;
		return theMap;
	}

	/**
	 * @return The only static instance of the map by message type
	 */
	static str_map &get_str_map()
	{
		static str_map theMap;
		return theMap;
	}

	/**
	 * A template function to build a new allocator for the message type and add it to both allocator maps
	 */
	template <class Msg>
	inline void add_allocator_to_maps()
	{
		message_allocator_impl<Msg> *alloc = new message_allocator_impl<Msg>();
		byInt[alloc->getMessageID()] = alloc;
		byStr[alloc->getMessageType()] = alloc;
	}

	/**
	 * The variadic version of the initialize function that adds one message allocator, then
	 * recurses across the others.
	 */
	template <class... Others, class Msg>
	inline void initialize_maps()
	{
		add_allocator_to_maps<Msg>();
		initialize_maps<Others...>();	// Recursive call
	}

	/// An object to hold any error messages
	std::unique_ptr<J2735Exception> error_message;
public:
	/**
	 * Creates a new factory to use.  The allocator maps will only be initialized once.
	 */
	J2735MessageFactory():
		byInt(get_int_map()),
		byStr(get_str_map())
	{
		if (byInt.size() <= 0 || byStr.size() <= 0)
		{
			add_allocator_to_maps<BsmMessage>();
#if SAEJ2735_SPEC < 63
			add_allocator_to_maps<BsmvMessage>();
#endif
			add_allocator_to_maps<CsrMessage>();
			add_allocator_to_maps<EvaMessage>();
			add_allocator_to_maps<IntersectionCollisionMessage>();
			add_allocator_to_maps<MapDataMessage>();
			add_allocator_to_maps<NmeaMessage>();
			add_allocator_to_maps<PdmMessage>();
#if SAEJ2735_SPEC < 63
			add_allocator_to_maps<PmmMessage>();
#endif
			add_allocator_to_maps<PvdMessage>();
			add_allocator_to_maps<PsmMessage>();
			add_allocator_to_maps<RsaMessage>();
			add_allocator_to_maps<RtcmMessage>();
			add_allocator_to_maps<SrmMessage>();
			add_allocator_to_maps<SsmMessage>();
			add_allocator_to_maps<SpatMessage>();
			add_allocator_to_maps<TimMessage>();
			add_allocator_to_maps<tsm4Message>();
			add_allocator_to_maps<tsm5Message>();
#if SAEJ2735_SPEC < 63
			add_allocator_to_maps<UperFrameMessage>();
#endif
		}
	}

	~J2735MessageFactory()	{ }

	/**
	 * Return the last event that occurred.  This can be used to report on the
	 * error that happened while creating the message
	 * @return The last event, or an empty event if none occurred
	 */
	inline J2735Exception get_event()
	{
		if (error_message)
			return *error_message;
		else
			return J2735Exception("No event");
	}

	/**
	 * Initialize the J2735 routeable message with the given byte stream.
	 *
	 * @param The J2735 message to initialize
	 * @param The byte stream
	 */
	void initialize(TmxJ2735EncodedMessageBase *msg, const tmx::byte_stream &bytes)
	{
		if (msg)
		{
			msg->set_data(bytes);
		}
	}

	/**
	 * Create a new J2735 Message from the given message ID.
	 *
	 * @param messageId The J2735 message ID
	 * @return A new message of that type, or NULL if an error occurs
	 */
	inline TmxJ2735EncodedMessageBase *NewMessage(int messageId)
	{
		error_message.reset();

		try
		{
			return byInt.at(messageId)->allocate();
		}
		catch (std::exception &ex)
		{
			error_message.reset(new J2735Exception(ex));
			*error_message << errmsg_info{"Unable to create new J2735 message from message ID " + std::to_string(messageId)};
			return NULL;
		}
	}

	/**
	 * Create a new J2735 Message from the given message type.
	 *
	 * @param messageType The type name for the J2735 Message
	 * @return A new message of that type, or NULL if an error occurs
	 */
	inline TmxJ2735EncodedMessageBase *NewMessage(std::string messageType)
	{
		error_message.reset();

		try
		{
			return byStr.at(messageType)->allocate();
		}
		catch (std::exception &ex)
		{
			error_message.reset(new J2735Exception(ex));
			*error_message << errmsg_info{"Unable to create new J2735 message from message type " + messageType};
			return NULL;
		}
	}

	inline int GetMessageId(std::string messageType)
	{
		error_message.reset();

		try
		{
			return byStr.at(messageType)->getMessageID();
		}
		catch (std::exception &ex)
		{
			error_message.reset(new J2735Exception(ex));
			*error_message << errmsg_info{"Unable to find J2735 message ID from message type " + messageType};
			return -1;
		}
	}

	inline int GetMessageId(tmx::byte_stream &bytes)
	{
		std::string unused;
		return GetCodecAndMessageId(unused, bytes);
	}

	inline const char *MessageType(int messageId)
	{
		error_message.reset();

		try
		{
			return byInt.at(messageId)->getMessageType();
		}
		catch (std::exception &ex)
		{
			error_message.reset(new J2735Exception(ex));
			*error_message << errmsg_info{"Unable to find J2735 message type from message ID " + std::to_string(messageId)};
			return NULL;
		}
	}

	/*
	 * Function to check for extended bytes and if found, strip them out for valid ASN.1 encoded messages
	 * Looks for prepended extra bytes 
	 * Understands the extended meaning and length fields
	 * Strips them out to create a valid hex string 
	 * returns the byte vector with valid hex string 

	*/

     inline int TmxJ2735ExtendedBytes(tmx::byte_stream &bytes)
	{	

		// for(int i=0;i<bytes.size();i++)
		// 	std::cout<<bytes[i]; 
		// std::cout<<std::endl;


		int msgIdindex=0; // msgId = DD
		if (bytes[0] == 0x00)
		{
			std::cout<<"No Extended bytes found\n"; 
			return 0; 
		}
		if(bytes[0] == 0x03){ // extended bytes present 
			if(bytes[1] == 0x00) // length < 64 bytes [ 0300DD]
				msgIdindex = 1;
			if(bytes[1] == 0x80){ //  
				if(bytes[2] == 0x81) // 128 < length < 256 [ 038081XX00DD ]
					msgIdindex = 4; 
				else if (bytes[2] == 0x82) // length > 256 [ 038082XXXX00DD ]
					msgIdindex = 5; 
				else // 64 < length < 128 bytes  [0380XX00DD] 
					msgIdindex = 3; 
			}
			std::cout<<"Extended bytes found\n"; 
		}

		bytes.erase(bytes.begin(),bytes.begin()+msgIdindex);

		// for(int i=0;i<bytes.size();i++)
		// 	std::cout<<bytes[i]; 
		// std::cout<<std::endl;

		return msgIdindex; 


	}


	/**
	 * Create a new J2735 Routeable Message from the given byte stream.  The message ID is separated
	 * from the byte stream to determine what message type to create.
	 *
	 * @param bytes The DER encoded set of bytes
	 * @return A new message of that type, or NULL if an error occurs
	 */
	inline TmxJ2735EncodedMessageBase *NewMessage(tmx::byte_stream &bytes)
	{
		error_message.reset();

		// look for extra bytes in the byte_stream 
		int msgidindex;
		if(msgidindex = TmxJ2735ExtendedBytes(bytes))
		{
			std::string byteStr(bytes.begin(),bytes.end());
			std::cout<<" Return = "<< msgidindex<< ",   Extended bytes found, sanitized::";
		}


		std::string codec;
		int id = GetCodecAndMessageId(codec, bytes);
		if (id > 0)
		{
			TmxJ2735EncodedMessageBase *msg = NewMessage(id);
			if (msg)
			{
				initialize(msg, bytes);
				if (!codec.empty())
					msg->set_encoding(codec);

				return msg;
			}
		}

		error_message.reset(new J2735Exception("Failed to create new J2735 message"));
		*error_message << errmsg_info{"Unable to determine message ID from bytes: " +
			battelle::attributes::attribute_lexical_cast<std::string>(bytes)};
		return NULL;
	}

private:
	inline int GetCodecAndMessageId(std::string &codec, tmx::byte_stream &bytes)
	{
		error_message.reset();

		// Always try the default encoding first
		ASN1_CODEC<MessageFrameMessage> Codec;
#if SAEJ2735_SPEC < 63
		tmx::messages::codec::uper<MessageFrameMessage> OtherCodec;
#else
		tmx::messages::codec::der<MessageFrameMessage> OtherCodec;
#endif

		int id = Codec.decode_contentId(bytes);
		std::cout<<" Codec ID  = "<< id <<std::endl;

		if (id < 0)
		{
			id = OtherCodec.decode_contentId(bytes);
			if (id > 0)
				codec.assign(OtherCodec.Encoding);
		}
		else
		{
			codec.assign(Codec.Encoding);
		}


		return id;

	}
};

} /* End namespace messages */
} /* End namespace tmx */

#endif /* TMX_J2735_MESSAGES_J2735MESSAGEFACTORY_HPP_ */
