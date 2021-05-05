/*
 * J2735MessageTemplate.hpp
 *
 *  Created on: Aug 24, 2017
 *      Author: gmb
 */

#ifndef TMX_J2735_MESSAGES_J2735MESSAGETEMPLATE_HPP_
#define TMX_J2735_MESSAGES_J2735MESSAGETEMPLATE_HPP_

#define TMX_J2735_NAMESPACE_START(X) \
namespace X {

#define TMX_J2735_NAMESPACE_END(X) \
} /* End namespace X */

#define TMX_J2735_DECLARE_TRAITS(T, ID, TYPE) \
template <> \
struct SaeJ2735Traits<T> { \
	typedef T message_type; \
	typedef asn_TYPE_descriptor_t asn_type; \
	static constexpr const int messageId = (int)ID; \
	static constexpr const asn_type &descriptor = asn_DEF_ ##T; \
	static constexpr const char *messageType = TYPE; \
};

#define TMX_J2735_TYPEDEF_TRAITS(T, NM) typedef j2735::SaeJ2735Traits<T> NM ## Traits;
#define TMX_J2735_TYPEDEF_MSG(NM) typedef TmxJ2735Message< typename NM ## Traits::message_type > NM ## Message;
#define TMX_J2735_TYPEDEF_ENCMSG(NM) typedef TmxJ2735EncodedMessage< NM ## Message > NM ## EncodedMessage;

#define TMX_J2735_ADD_NAMESPACE(X, Y) X::Y
#define TMX_J2735_SPECIALIZE_GET_PAYLOAD(NM) \
template <> template <> \
inline TMX_J2735_ADD_NAMESPACE(messages, NM ## Message) \
routeable_message::get_payload<TMX_J2735_ADD_NAMESPACE(messages, NM ## Message)>() \
{ \
	TMX_J2735_ADD_NAMESPACE(messages, NM ## EncodedMessage) encMsg(*this); \
	return encMsg.get_payload<TMX_J2735_ADD_NAMESPACE(messages, NM ## Message)>(); \
}

#if SAEJ2735_SPEC < 63
#define TMX_J2735_MESSAGE_FRAME_FROM(T) \
template <> \
inline T *_j2735_cast< SaeJ2735Traits<T>, MessageFrameTraits>\
	(const typename MessageFrameTraits::message_type *ptr) \
{ return tmx::messages::j2735::UperFrameDecode<T>(ptr); }

#define TMX_J2735_MESSAGE_FRAME_TO(T) \
template <> \
inline typename MessageFrameTraits::message_type *\
	_j2735_cast< MessageFrameTraits, SaeJ2735Traits<T> >(const T *ptr) \
{ return tmx::messages::j2735::UperFrameEncode<T>(ptr); }

#else
#define TMX_J2735_MESSAGE_FRAME_FROM(T) \
template <> \
inline T *_j2735_cast< SaeJ2735Traits<T>, MessageFrameTraits>\
	(const typename MessageFrameTraits::message_type *ptr) \
{ \
	if (ptr && ptr->value.present == value_PR_ ##T) \
		return (T *)&(ptr->value.choice.T); \
	else \
		return 0; \
}

#define TMX_J2735_MESSAGE_FRAME_TO(T) \
template <> \
inline typename MessageFrameTraits::message_type *\
	_j2735_cast< MessageFrameTraits, SaeJ2735Traits<T> >(const T *ptr) \
{ \
	if (ptr) \
	{ \
		typename MessageFrameTraits::message_type *frame = \
			(typename MessageFrameTraits::message_type *)\
			calloc(1, sizeof(typename MessageFrameTraits::message_type)); \
		frame->messageId = get_default_messageId< SaeJ2735Traits<T> >(); \
		frame->value.present = value_PR_ ##T; \
		frame->value.choice.T = *(ptr); \
		return frame; \
	} \
	return 0; \
}

#endif

#define TMX_J2735_DECLARE_MESSAGE(NM, T, ID, TYPE) \
TMX_J2735_NAMESPACE_START(tmx) \
TMX_J2735_NAMESPACE_START(messages) \
TMX_J2735_NAMESPACE_START(j2735) \
TMX_J2735_DECLARE_TRAITS(T, ID, TYPE) \
TMX_J2735_NAMESPACE_END(j2735) \
TMX_J2735_TYPEDEF_TRAITS(T, NM) \
TMX_J2735_TYPEDEF_MSG(NM)

#define TMX_J2735_DECLARE_END \
TMX_J2735_NAMESPACE_END(messages) \
TMX_J2735_NAMESPACE_END(tmx)

#define TMX_J2735_DECLARE(NM, T, ID, TYPE) \
TMX_J2735_DECLARE_MESSAGE(NM, T, ID, TYPE) \
TMX_J2735_TYPEDEF_ENCMSG(NM) \
TMX_J2735_NAMESPACE_START(j2735) \
TMX_J2735_MESSAGE_FRAME_FROM(T) \
TMX_J2735_MESSAGE_FRAME_TO(T) \
TMX_J2735_NAMESPACE_END(j2735) \
TMX_J2735_NAMESPACE_END(messages) \
TMX_J2735_SPECIALIZE_GET_PAYLOAD(NM) \
TMX_J2735_NAMESPACE_END(tmx)

#endif /* TMX_J2735_MESSAGES_J2735MESSAGETEMPLATE_HPP_ */
