/*
 * TmxJ2735Codec.hpp
 *
 *  Created on: Sep 16, 2016
 *      Author: gmb
 */

#ifndef TMX_MESSAGES_TMXJ2735CODEC_HPP_
#define TMX_MESSAGES_TMXJ2735CODEC_HPP_

#include <string>
#include <tmx/messages/TmxJ2735.hpp>

#define TMX_J2735_MAX_DATA_SIZE 4000

#define ASN1_CODEC tmx::messages::codec::uper

#include <tmx/j2735_messages/MessageFrame.hpp>

namespace tmx {
namespace messages {

typedef boost::error_info<struct tag_codec, std::string> codecerr_info;

namespace codec {

/**
 * A structure for ASN.1 DER encoding/decoding, a type of Basic Encoding rules (BER).
 * Attached to encoding type "asn.1-ber/hexstring"
 */
template <typename MsgType>
struct der
{
	typedef MsgType type;
	typedef typename type::message_type message_type;
	typedef tmx::messages::codec::der<type> codec;

	static constexpr const char *Encoding = api::ENCODING_ASN1_BER_STRING;

	/**
	 * Decode from the byte stream to an ASN.1 struct pointer for the specified message type.
	 * Note that this function skips up to 16 bytes ahead, looking for the start of the
	 * DER encoded bytes.
	 * @param obj The struct pointer to decode to.  Memory is malloc'd, thus must be freed
	 * @param bytes The bytes to decode
	 * @param typeDescriptor The ASN.1 struct type descriptor for the message type
	 * @return The decode status return
	 */
	asn_dec_rval_t decode(void **obj, const tmx::byte_stream &bytes,
			asn_TYPE_descriptor_t *typeDescriptor = MsgType::get_descriptor())
	{
		size_t start;
		int count = 0;
		for (start = 0; start < bytes.size() && bytes[start] != 0x30 && count <= 16; start++, count++);
		if (count > 15) start = 0;  // Beginning not found, just try it
		return ber_decode(0, typeDescriptor, obj, &bytes.data()[start], bytes.size() - start);
	}

	/**
	 * Encode the ASN.1 struct pointer for the specified message type to a byte stream
	 * @param obj The struct pointer to encode
	 * @param bytes The bytes to encode to with enough buffer space pre-allocated
	 * @param typeDescriptor The ASN.1 struct type descriptor for the message type
	 * @return The encode status return
	 */
	asn_enc_rval_t encode(const typename MsgType::message_type *obj, tmx::byte_stream &bytes,
			asn_TYPE_descriptor_t *typeDescriptor = MsgType::get_descriptor())
	{
		return der_encode_to_buffer(typeDescriptor, (void *)obj, bytes.data(), bytes.max_size());
	}

	/**
	 * Attempt to pull out the message ID from the DER encoded bytes.  This is done by
	 * studying the byte contents, and looking at the 5th segment.  Note that this only
	 * works for pre-2016 message types which encoded the message ID in the DER bytes.
	 * Note also that this function skips up to 16 bytes ahead, looking for the start of the
	 * DER encoded bytes.
	 * @param bytes The bytes to decode
	 * @return The message ID enclosed in this DER encoding, or < 0 if none can be found
	 */
	static int decode_contentId(const tmx::byte_stream &bytes)
	{
		int count = 0;

		// Read the DER encoded bytes to obtain the message ID
		byte_stream::const_iterator it;
		for (it = bytes.begin(); it != bytes.end() && *it != 0x30 && count <= 16; it++, count++);
		if (count < 16)
		{
			// Should be the 5th segment
			ber_tlv_len_t len;
			ssize_t lenBytes = 1;
			for (int idx = count, count = 0; count < 5 && idx <= (int)bytes.size(); idx += lenBytes, count++)
			{
				lenBytes = ber_fetch_length(1, &(bytes.data()[idx]), bytes.size() - idx, &len);
				if (lenBytes == 0 || lenBytes == -1)
				{
					len = 0;
					break;
				}

				// For framed messages, the content ID comes 2 bytes after
				if (len == (int)api::uperFrame_D)
					count--;
			}

			// For some reason, the UPER frame encoded content ID shows up as the second to last byte
			if (lenBytes > 1)
				len = (len >> 8) & 0xFF;

			if (len > api::J2735 && len < api::J2735_end)
				return static_cast<int>(len);
		}

		return -1;
	}
};

/**
 * A structure for ASN.1 UPER encoding/decoding.  Attached to encoding type "asn.1-uper/hexstring"
 */
template <typename MsgType>
struct uper
{
	typedef MsgType type;
	typedef typename type::message_type message_type;
	typedef tmx::messages::codec::uper<type> codec;

	static constexpr const char *Encoding = api::ENCODING_ASN1_UPER_STRING;

	/**
	 * Decode from the byte stream to an ASN.1 struct pointer for the specified message type.
	 * @param obj The struct pointer to decode to.  Memory is malloc'd, thus must be freed
	 * @param bytes The bytes to decode
	 * @param typeDescriptor The ASN.1 struct type descriptor for the message type
	 * @return The decode status return
	 */
	asn_dec_rval_t decode(void **obj, const tmx::byte_stream &bytes,
			asn_TYPE_descriptor_t *typeDescriptor = MsgType::get_descriptor())
	{
		return uper_decode_complete(0, typeDescriptor, obj, bytes.data(), bytes.size());
	}

	/**
	 * Encode the ASN.1 struct pointer for the specified message type to a byte stream
	 * @param obj The struct pointer to encode
	 * @param bytes The bytes to encode to with enough buffer space pre-allocated
	 * @param typeDescriptor The ASN.1 struct type descriptor for the message type
	 * @return The encode status return
	 */
	asn_enc_rval_t encode(const typename MsgType::message_type *obj, tmx::byte_stream &bytes,
			asn_TYPE_descriptor_t *typeDescriptor = MsgType::get_descriptor())
	{
		void * t; 
		asn_enc_rval_t ueRet = uper_encode_to_buffer(typeDescriptor,(asn_per_constraints_t*) t, (void *)obj, bytes.data(), bytes.max_size());

		// For UPER encoding, the number of bytes encoded must be adjusted
		ueRet.encoded = (ueRet.encoded + 7) / 8;
		return ueRet;
	}

	/**
	 * Attempt to pull out the content ID from the UPER encoded bytes.  This is done by
	 * decoding the message frame and looking at the contained ID in the frame.
	 * @param bytes The bytes to decode
	 * @return The message ID enclosed in this DER encoding, or < 0 if none can be found
	 * TODO Try to scan the bytes in the frame instead of decoding all the way
	 */
	static int decode_contentId(const tmx::byte_stream &bytes)
	{
		int id;

		MessageFrameMessage::message_type *frame = NULL;
		asn_TYPE_descriptor_t *descriptor = NULL;
		// Message ID is encoded directly in the message frame, first two bytes
		if (bytes.size() > 1)
			id = bytes[0];

		if (bytes.size() > 2) {
			id <<= 8;
			id |= bytes[1];
		}

		if (descriptor)
			ASN_STRUCT_FREE((*descriptor), frame);

		return id;
	}

};

} /* End namespace codec */

/**
 * A base class for all J2735 messages
 */
class TmxJ2735EncodedMessageBase: public tmx::routeable_message
{
public:
	TmxJ2735EncodedMessageBase(IvpMessage *other = 0): tmx::routeable_message(other) {}
	TmxJ2735EncodedMessageBase(tmx::message_container_type &contents): tmx::routeable_message(contents) {}
	TmxJ2735EncodedMessageBase(const tmx::routeable_message &other): tmx::routeable_message(other) {}
	virtual ~TmxJ2735EncodedMessageBase() {}

	virtual tmx::xml_message get_payload() = 0;
	virtual int get_msgId() = 0;
	virtual int get_msgKey() = 0;

	tmx::byte_stream get_data()
	{
		return get_payload_bytes();
	}

	virtual void set_data(const tmx::byte_stream &data)
	{
		std::string enc = get_encoding();
		set_payload_bytes(data);
		set_encoding(enc);
	}
};

/**
 * A template class for an encoded J2735 message that can be routed through the TMX core.
 */
template <typename MsgType>
class TmxJ2735EncodedMessage: public TmxJ2735EncodedMessageBase
{
public:
	typedef tmx::messages::codec::uper<MsgType> UperCodec;
	typedef tmx::messages::codec::der<MsgType> DerCodec;

	static constexpr const char *DefaultCodec = ASN1_CODEC<MsgType>::Encoding;

	/**
	 * Construct an message with the optionally supplied IVP message.  The
	 * default codec is used for this type.
	 */
	TmxJ2735EncodedMessage(IvpMessage *other = 0): TmxJ2735EncodedMessageBase(other)
	{
		this->set_type(MsgType::MessageType);
		this->set_subtype(MsgType::MessageSubType);
		this->set_encoding(DefaultCodec);
	}

	/**
	 * Construct a message from a copy of another message of the same type
	 * @param other The other message
	 */
	TmxJ2735EncodedMessage(const TmxJ2735EncodedMessage<MsgType> &other):
		TmxJ2735EncodedMessageBase(other), _decoded(other._decoded ? other._decoded.get() : 0) { }

	/**
	 * Construct a message from a copy of another routeable message of a different type
	 * @param other The other message
	 */
	TmxJ2735EncodedMessage(tmx::routeable_message &other):
		TmxJ2735EncodedMessageBase(other) { }

	/**
	 * Construct a message from a message container
	 * @param contents The contents to load
	 */
	TmxJ2735EncodedMessage(const tmx::message_container_type &contents):
		TmxJ2735EncodedMessageBase(contents) { }
public:
	void set_data(const tmx::byte_stream &data)
	{
		// Clear out old cache
		_decoded.reset();

		// Set the data
		TmxJ2735EncodedMessageBase::set_data(data);

		// If DER encoding is detected, then force that encoding
		if (data.size() > 0 && data[0] == 0x30)
			this->set_encoding(codec::der<MsgType>::Encoding);
	}

	/**
	 * Decode the J2735 message from the given bytes using a specific decoder type, which must
	 * implement the decode() function.
	 * @param bytes The byte stream to decode
	 * @return The decoded J2735 message
	 */
	template <typename DecType>
	static typename DecType::type *decode_j2735_message(tmx::byte_stream bytes)
	{
		typedef typename DecType::type type;
		typedef typename DecType::message_type msg_type;

		DecType decoder;
		msg_type *obj = 0;
		asn_dec_rval_t rval = decoder.decode((void **)&obj, bytes);

		if (rval.code == RC_OK)
		{
			return new type(obj);
		}
		else
		{
			J2735Exception err("Unable to decode " +
					std::string(j2735::get_messageTag< typename type::traits_type >()) + " from bytes.");
			err << codecerr_info{DecType::Encoding};
			err << errmsg_info{"Failed after " + std::to_string(rval.consumed) + " bytes."};
			BOOST_THROW_EXCEPTION(err);
		}
	}

	/**
	 * Encode the given message to a byte stream using a specific encoder type, which must
	 * implement the encode() function.
	 * @param message The J2735 message to encode
	 * @returns The encoded byte stream
	 */
	template <typename EncType>
	static tmx::byte_stream encode_j2735_message(typename EncType::type &message)
	{
		typedef typename EncType::type type;

		EncType encoder;
		tmx::byte_stream bytes(TMX_J2735_MAX_DATA_SIZE);
		asn_enc_rval_t rval = encoder.encode(message.get_j2735_data().get(), bytes);

		if (rval.encoded <= 0)
		{
			J2735Exception err("Unable to encode " +
					std::string(j2735::get_messageTag< typename type::traits_type >()) + " to bytes.");
			err << codecerr_info{EncType::Encoding};
			err << errmsg_info{rval.failed_type->name};
			BOOST_THROW_EXCEPTION(err);
		}
		else
		{
			bytes.resize(rval.encoded);
			bytes.shrink_to_fit();
			return bytes;
		}
	}

	/**
	 * Decode the J2735 message from the data attribute using the default encoding type specified in
	 * the encoding attribute.
	 * @return The decoded J2735 message
	 */
	MsgType decode_j2735_message()
	{
		if (!_decoded)
		{
			const tmx::byte_stream &theData = this->get_data();
			int msgId = get_msgId();

			// If the encoding is incorrect for this J2735 specification, send an empty message
			if (strcmp(ASN1_CODEC<MessageFrameMessage>::Encoding, this->get_encoding().c_str()) != 0)
			{
				// Unable to decode
				_decoded.reset(new MsgType());
			}
			else if (is_uper())
			{
				if (msgId > MessageFrameMessage::get_default_messageId())
				{
					MessageFrameMessage *frame = TmxJ2735EncodedMessage<MessageFrameMessage>::decode_j2735_message<
							codec::uper<MessageFrameMessage> >(theData);
					if (frame)
						_decoded.reset(new MsgType(frame->get_j2735_data()));
				}
				else
				{
					_decoded.reset(TmxJ2735EncodedMessage<MsgType>::decode_j2735_message<UperCodec>(theData));
				}
			}
			else if (is_der())
			{
				if (msgId > MessageFrameMessage::get_default_messageId())
				{
					MessageFrameMessage *frame = TmxJ2735EncodedMessage<MessageFrameMessage>::decode_j2735_message<
							codec::der<MessageFrameMessage> >(theData);
					if (frame)
						_decoded.reset(new MsgType(frame->get_j2735_data()));
				}
				else
				{
					_decoded.reset(TmxJ2735EncodedMessage<MsgType>::decode_j2735_message<DerCodec>(theData));
				}
			}
			else
			{
				J2735Exception err("Unknown encoding.");
				err << codecerr_info{this->get_encoding()};
				BOOST_THROW_EXCEPTION(err);
				throw;	// Just to suppress the warning for non-return value
			}
		}

		return *_decoded;
	}

	/**
	 * Encode the given J2735 message to the data attribute using the default encoding type specified
	 * in the encoding attribute.
	 * @param message The J2735 message to encode
	 */
	void encode_j2735_message(MsgType &message)
	{
		int msgId = get_msgId();

		if (is_uper())
		{
			if (msgId > MessageFrameMessage::get_default_messageId())
			{
				MessageFrameMessage frame(message.get_j2735_data());
				frame.flush();
				this->set_data(TmxJ2735EncodedMessage<MsgType>::encode_j2735_message<
						codec::uper<MessageFrameMessage> >(frame));
			}
			else
			{
				this->set_data(TmxJ2735EncodedMessage<MsgType>::encode_j2735_message<UperCodec>(message));
			}
		}
		else if (is_der())
		{
			if (msgId > MessageFrameMessage::get_default_messageId())
			{
				MessageFrameMessage frame(message.get_j2735_data());
				this->set_data(TmxJ2735EncodedMessage<MsgType>::encode_j2735_message<
						codec::der<MessageFrameMessage> >(frame));
			}
			else
			{
				this->set_data(TmxJ2735EncodedMessage<MsgType>::encode_j2735_message<DerCodec>(message));
			}
		}
		else
		{
			J2735Exception err("Unknown encoding");
			err << codecerr_info{this->get_encoding()};
			BOOST_THROW_EXCEPTION(err);
		}
	}

	/**
	 * Override of the template function in tmx::routeable_message to ensure the correct message type
	 * is decoded and extracted.  A compiler error should occur if trying to extract an incompatible type.
	 * @return The decoded J2735 message
	 */
	template <typename OtherMsgType>
	OtherMsgType get_payload()
	{
		if (this->get_encoding().find_last_of("hexstring") > 0)
		{
			// Return the decoded message type
			return decode_j2735_message();
		}
		else
		{
			decode_j2735_message();
			_decoded->flush();

			return OtherMsgType(*_decoded);
		}
	}

	xml_message get_payload()
	{
		// Make sure the container is flushed
		decode_j2735_message();
		_decoded->flush();

		// Copy to a new XML message
		return *_decoded;
	}

	/**
	 * @return The message identifier for the encoded type
	 */
	int get_msgId()
	{
		int id = -1;

		if (is_uper())
		{
			id = UperCodec::decode_contentId(this->get_data());
		}
		else if (is_der())
		{
			id = DerCodec::decode_contentId(this->get_data());
		}

		if (id > 0)
			return id;
		else
			return MsgType::get_default_messageId();
	}

	int get_msgKey()
	{
		if (!_decoded)
			decode_j2735_message();

		return _decoded->get_messageKey();
	}

	/**
	 * Initialize the message data
	 * @param payload The J2735 message payload
	 * @param source The source of the message
	 * @param sourceId The source ID of the message
	 * @param flags Flags for the TMX router
	 */
	virtual void initialize(MsgType &payload, std::string source = "", unsigned int sourceId = 0, unsigned int flags = 0)
	{
		tmx::routeable_message::initialize(MsgType::MessageType, MsgType::MessageSubType,
				source, sourceId, flags);
		this->encode_j2735_message(payload);
	}
private:
	std::unique_ptr<MsgType> _decoded;

	template <typename EncType>
	bool is_encoded()
	{
		return (this->get_encoding() == EncType::Encoding);
	}

public:
	bool is_der()
	{
		return is_encoded<DerCodec>();
	}

	bool is_uper()
	{
		return is_encoded<UperCodec>();
	}
};

typedef TmxJ2735EncodedMessage<MessageFrameMessage> MessageFrameEncodedMessage;

#if SAEJ2735_SPEC < 2016
namespace j2735 {

// Decode and encode for UPER frame
template <typename T>
T *UperFrameDecode(const typename MessageFrameMessage::message_type *ptr)
{
	if (ptr && ptr->contentID == get_default_messageId< SaeJ2735Traits<T> >())
	{
		tmx::messages::codec::uper< TmxJ2735Message<T> > uperDecoder;
		tmx::byte_stream uperFrameBytes(ptr->msgBlob.size);
		memcpy(uperFrameBytes.data(), ptr->msgBlob.buf, ptr->msgBlob.size);
		T *obj = 0;
		asn_dec_rval_t udRet = uperDecoder.decode((void **)obj, uperFrameBytes);
		if (udRet.code == RC_OK && obj)
			return obj;
	}

	return 0;
}

template <typename T>
typename MessageFrameMessage::message_type *UperFrameEncode(const T *ptr)
{
	if (ptr)
	{
		tmx::messages::codec::uper< TmxJ2735Message<T> > uperEncoder;
		tmx::byte_stream uperFrameBytes(TMX_J2735_MAX_DATA_SIZE);
		asn_enc_rval_t ueRet = uperEncoder.encode(ptr, uperFrameBytes);
		if (ueRet.encoded > 0)
		{
			typename MessageFrameMessage::message_type *uperFrame =
					(typename MessageFrameMessage::message_type *)
						calloc(1, sizeof(typename MessageFrameMessage::message_type));

			uperFrameBytes.resize(ueRet.encoded);

			uperFrame->msgID = get_default_messageId< MessageFrameTraits >();
			uperFrame->contentID = get_default_messageId< SaeJ2735Traits<T> >();
			uperFrame->msgBlob.buf = (uint8_t *) calloc(ueRet.encoded, sizeof(uint8_t));
			uperFrame->msgBlob.size = (int)ueRet.encoded;
			::memcpy(uperFrame->msgBlob.buf, uperFrameBytes.data(), ueRet.encoded);
			return uperFrame;
		}
	}

	return 0;
}

} /* End namespace j2735 */
#endif

} /* End namespace messages */
} /* End namespace tmx */


#endif /* TMX_MESSAGES_TMXJ2735CODEC_HPP_ */
