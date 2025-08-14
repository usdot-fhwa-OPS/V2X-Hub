/*
 * TmxJ2735.hpp
 *
 *  Created on: Apr 27, 2016
 *      Author: ivp
 */

#ifndef TMX_MESSAGES_TMXJ2735_HPP_
#define TMX_MESSAGES_TMXJ2735_HPP_

#include <cerrno>
#include <memory>
#include <stdexcept>
#include <stdio.h>

#include <asn_application.h>
#include <xer_encoder.h>
#include <jer_encoder.h>
#include <asn_internal.h>
#include <boost/any.hpp>
#include <tmx/TmxApiMessages.h>
#include <tmx/messages/J2735Exception.hpp>
#include <tmx/messages/SaeJ2735Traits.hpp>
#include <tmx/messages/routeable_message.hpp>
#include <tmx/messages/message.hpp>
namespace tmx {
namespace messages {
namespace j2735 {

/*
 * Return a unique key for the given message, which by default is 0.  If a message can
 * be identified by a unique key, then it should specialize this function.
 *
 * @return An integer unique identifier for the supplied J2735 message, or 0 if no key can be identified.
 */
template <typename MsgType>
int get_j2735_message_key(std::shared_ptr<typename MsgType::message_type> message) { return 0; }

} /* End namespace j2735 */


/**
 * A template class for all J2735 messages.  This class is a decoded version of the
 * specific J2735 data type structure built with the ASN.1 compiler, but represented
 * in a boost::property_tree.  This type can be used in a handler if you want the
 * decode version of the message, which would make the most sense.
 */
template <typename DataType, typename Format = XML>
class TmxJ2735Message: public tmx::tmx_message<Format>
{
public:
	/// The J2735 data type
	typedef j2735::SaeJ2735Traits<DataType> traits_type;
	typedef typename traits_type::message_type message_type;
	typedef typename traits_type::asn_type asn_type;
	typedef TmxJ2735Message<DataType> type;

	/**
	 * @return The ASN.1 descriptor for the J2735 data type
	 */
	static asn_type *get_descriptor()
	{
		static constexpr const asn_type *descriptor = j2735::get_descriptor<traits_type>();
		asn_type *ret = (asn_type *)descriptor;
		if (!ret)
			BOOST_THROW_EXCEPTION(J2735Exception("Null ASN descriptor type discovered for " +
				battelle::attributes::type_id_name<message_type>() + "."));
		return ret;
	}

	static constexpr const int get_default_messageId() {
		return j2735::get_default_messageId<traits_type>();
	}

	static constexpr const char *get_messageTag() {
		return j2735::get_messageTag<traits_type>();
	}

	static constexpr const char *get_messageType() {
		return j2735::get_messageType<traits_type>();
	}

	static constexpr const char *MessageType = tmx::messages::api::MSGSUBTYPE_J2735_STRING;
	static constexpr const char *MessageSubType = get_messageType();

	/**
	 * Create a J2735 Message of this type, optionally using a pointer to the J2735
	 * data type structure.  If no pointer is given, then the message is empty until
	 * contents can be loaded in some other manner.
	 * @param data Pointer to the J2735 data
	 */
	TmxJ2735Message(message_type *data = 0):
		tmx::tmx_message<Format>(),
		_j2735_data(data, [](message_type *p) { j2735::j2735_destroy<traits_type>(p); } ) { }

	/**
	 * Copy constructor
	 */
	TmxJ2735Message(const type& msg):
		tmx::tmx_message<Format>(msg), _j2735_data(msg._j2735_data) { }

	/**
	 * Copy from an existing reference to the message type.  This expects that all
	 * memory management is done on the actual reference, so no clean up is done.
	 */
	TmxJ2735Message(message_type &msg):
		tmx::tmx_message<Format>(), _j2735_data(&msg, [](message_type *p) { }) { }

	/**
	 * Copy from existing shared pointer of same type.  Current ownership is still
	 * maintained in the existing shared pointer, but reference count is increased.
	 */
	TmxJ2735Message(const std::shared_ptr<message_type> &other):
		tmx::tmx_message<Format>(), _j2735_data(other) { }

	/**
	 * Same as above, but copy from a different message type, presumably
	 * one that is convertible to this type.  A new reference to the ASN.1 struct
	 * message pointer is created, but the original owner still maintains ownership.
	 */
	template <typename OtherMsgType>
	TmxJ2735Message(const std::shared_ptr<OtherMsgType> &other):
		tmx::tmx_message<Format>(),
		_j2735_data(j2735::j2735_cast<message_type>(other.get()), [](message_type *p) { }) { }

	template <typename OtherFormat>
	TmxJ2735Message(const TmxJ2735Message<OtherFormat> &other, message_converter *converter = 0):
		tmx::tmx_message<Format>(other.get_container())
	{
		
	}

	/**
	 * Destructor
	 */
	virtual ~TmxJ2735Message() { }

	/*
	 * Define assignment operator to keep compiler from creating a default which
	 * will copy the _j2735_data pointer which can cause exceptions
	 */
	type& operator=(const type &msg)
	{
		if (this != &msg)
		{
			_j2735_data.reset();
			_j2735_data = msg._j2735_data;
			tmx::tmx_message<Format>::operator=(msg);
		}
		return *this;
	}

	message_container_type get_container() const
	{
		if (!_j2735_data)
			return tmx_message<Format>::get_container();

		// Make a copy of the current container and serialize to it
		message_container_type copy(tmx_message<Format>::get_container());
		copy.load<Format>(as_string(*_j2735_data));
		return copy;
	}

	/**
	 * Returns a pointer to a filled in J2735 data structure, taken from an message serialization of the property tree.
	 * @return The pointer to the structure
	 */
	std::shared_ptr<message_type> get_j2735_data()
	{
		if (!_j2735_data && !tmx::tmx_message<Format>::is_empty())
		{
			message_type *tmp = 0;
			std::string myData = this->to_string();

			asn_dec_rval_t rval;
			if (tmx_message<Format>::is_format("XML")) {
				rval = xer_decode(NULL, get_descriptor(), (void **)&tmp, myData.c_str(), myData.size());
				if (rval.code != RC_OK)
				{
					std::stringstream err;
					err << "Unable to decode " << MessageSubType << " from " << myData <<
							// " in format " << tmx::tmx_message<Format>::format() <<
							"\nFailed after " << rval.consumed << " bytes.";
					BOOST_THROW_EXCEPTION(J2735Exception(err.str()));
				}

				_j2735_data.reset(tmp);
			}else if (tmx_message<Format>::is_format("JSON")) {
				rval = jer_decode(NULL, get_descriptor(), (void **)&tmp, myData.c_str(), myData.size());
				if (rval.code != RC_OK)
				{
					std::stringstream err;
					err << "Unable to decode " << MessageSubType << " from " << myData <<
							"\nFailed after " << rval.consumed << " bytes.";
					BOOST_THROW_EXCEPTION(J2735Exception(err.str()));
				}

				_j2735_data.reset(tmp);
			} else {
				BOOST_THROW_EXCEPTION(J2735Exception("Unsupported format for J2735 message: " + tmx::tmx_message<Format>::format()));
			}
			
		}

 		return _j2735_data;
	}

	/**
	 * Sets the J2735 data structure by serializing the given data pointer into the XML/JSON container,
	 * and then re-constructing a new copy of the data pointer.  The supplied pointer is <b>not</b>
	 * managed by this object.
	 *
	 * @param data A pointer to a filled in J2735 data structure
	 */
	void set_j2735_data(const message_type *data)
	{
		_j2735_data.reset();
		tmx_message<Format>::clear();

		if (data)
			set_contents(as_string<message_type>(*data));
	}

	using tmx_message<Format>::flush;

	int get_messageKey() {
		return j2735::get_j2735_message_key<type>(get_j2735_data());
	}
	std::string as_string() const
	{
		if (_j2735_data)
			return as_string(*_j2735_data, get_descriptor());
		else
			return "";
	}
protected:
	/**
	 * Populates the property tree from an serialization dependent on message format (JSON/XML) of the supplied J2735 data structure
	 */
	virtual void flush(message_container_type &container) const
	{
		if (_j2735_data)
		{
			std::stringstream ss;
			ss << as_string(*_j2735_data);
			container.load<Format>(ss);
		}
	}


	std::shared_ptr<message_type> _j2735_data;
private:

	using buffer_structure_t = struct buffer_structure
    {
        char *buffer;          // buffer array
        size_t buffer_size;    // this is really where we will write next.
        size_t allocated_size; // this is the total size of the buffer.
    };

	static int DynamicBufferAppend(const void *buffer, size_t size, void *app_key)
    {
        auto *xb = static_cast<buffer_structure_t *>(app_key);

        while (xb->buffer_size + size + 1 > xb->allocated_size)
        {
            // increase size of buffer.
            size_t new_size = 2 * (xb->allocated_size ? xb->allocated_size : 64);
            auto new_buf = static_cast<char *>(MALLOC(new_size));
            if (!new_buf)
                return -1;
            // move old to new.
            memcpy(new_buf, xb->buffer, xb->buffer_size);

            FREEMEM(xb->buffer);
            xb->buffer = new_buf;
            xb->allocated_size = new_size;
        }

        memcpy(xb->buffer + xb->buffer_size, buffer, size);
        xb->buffer_size += size;
        // null terminate the string.
        xb->buffer[xb->buffer_size] = '\0';
        return 0;
    }

	template <typename T = message_type>
	std::string as_string(const T &data, asn_type *descr = get_descriptor()) const
	{
		buffer_structure_t string_buffer = {nullptr, 0, 0};
        
        
		
		if (tmx_message<Format>::is_format("XML"))
		{
			asn_enc_rval_t encode_rval = xer_encode(
				descr,
				&data,
				xer_encoder_flags_e::XER_F_CANONICAL,
				DynamicBufferAppend,
				static_cast<void *>(&string_buffer));
			if (encode_rval.encoded == -1)
			{
				BOOST_THROW_EXCEPTION(J2735Exception("Unable to stream XML contents in memory: Unknown error"))	;		
			}
			
		}
		else if (tmx_message<Format>::is_format("JSON"))
		{
			asn_enc_rval_t encode_rval = jer_encode(
				descr,
				&data,
				jer_encoder_flags_e::JER_F_MINIFIED,
				DynamicBufferAppend,
				static_cast<void *>(&string_buffer));
			if (encode_rval.encoded == -1)
			{
				BOOST_THROW_EXCEPTION(J2735Exception("Unable to stream JSON contents in memory: Unknown error"));
			}
		}
		else
		{
			FREEMEM(string_buffer.buffer);
			BOOST_THROW_EXCEPTION(J2735Exception("Unsupported format for J2735 message: " + tmx::tmx_message<Format>::format()));
		}
		
		auto output = std::string(string_buffer.buffer);
        return output;
	}
};

} /* End namespace messages */
} /* End namespace tmx */

// Automatically include the encoded and decoded messages
#include <tmx/messages/TmxJ2735Codec.hpp>

#endif /* TMX_MESSAGES_TMXJ2735_HPP_ */
