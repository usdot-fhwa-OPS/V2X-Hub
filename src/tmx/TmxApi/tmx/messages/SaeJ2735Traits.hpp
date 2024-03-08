/*
 * SaeJ2735Traits.hpp
 *
 *  Created on: Aug 18, 2017
 *      Author: gmb
 */

#ifndef TMX_MESSAGES_SAEJ2735TRAITS_HPP_
#define TMX_MESSAGES_SAEJ2735TRAITS_HPP_

#include <tmx/attributes/type_basics.hpp>
#include <tmx/messages/J2735Exception.hpp>
#include <type_traits>
#include <utility>

#ifndef SAEJ2735_SPEC
#define SAEJ2735_SPEC 2024
#endif

#define DEFINE_MEMBER_CHECKER(member) \
    template<typename T, typename V = bool> \
    struct has_ ## member : std::false_type { }; \
    template<typename T> \
    struct has_ ## member<T, \
        typename std::enable_if< \
            !std::is_same<decltype(std::declval<T>().member), void>::value, \
            bool \
            >::type \
        > : std::true_type { };

#define HAS_MEMBER(C, member) \
    has_ ## member<C>::value

namespace tmx {
namespace messages {
namespace j2735 {

DEFINE_MEMBER_CHECKER(messageId)
DEFINE_MEMBER_CHECKER(descriptor)
DEFINE_MEMBER_CHECKER(messageTag)
DEFINE_MEMBER_CHECKER(messageType)

template <typename T>
struct SaeJ2735Traits { };

template <typename TraitsType>
static constexpr typename std::enable_if<HAS_MEMBER(TraitsType, messageId), const int>::type
get_default_messageId() {
	return TraitsType::messageId;
}

template <typename TraitsType>
static constexpr typename std::enable_if<!HAS_MEMBER(TraitsType, messageId), const int>::type
get_default_messageId() {
	return -1;
}

template <typename TraitsType>
static constexpr typename std::enable_if<HAS_MEMBER(TraitsType, descriptor), const typename TraitsType::asn_type *>::type
get_descriptor() {
	return &TraitsType::descriptor;
}

template <typename TraitsType>
static constexpr typename std::enable_if<!HAS_MEMBER(TraitsType, descriptor), const typename TraitsType::asn_type *>::type
get_descriptor() {
	return 0;
}

template <typename TraitsType>
static constexpr typename std::enable_if<HAS_MEMBER(TraitsType, messageType), const char *>::type
get_messageType()
{
	return TraitsType::messageType;
}

template <typename TraitsType>
static constexpr typename std::enable_if<!HAS_MEMBER(TraitsType, messageType), const char *>::type
get_messageType()
{
	return "Unknown";
}

template <typename TraitsType>
static constexpr typename std::enable_if<HAS_MEMBER(TraitsType, messageTag), const char *>::type
get_messageTag()
{
	return TraitsType::messageTag;
}

template <typename TraitsType>
static constexpr typename std::enable_if<!HAS_MEMBER(TraitsType, messageTag), const char *>::type
get_messageTag()
{
	return get_descriptor<TraitsType>() != 0 ?
			get_descriptor<TraitsType>()->xml_tag :
			get_messageType<TraitsType>();
}

template <typename TraitsType>
static void j2735_destroy(typename TraitsType::message_type *ptr,
						  const typename TraitsType::asn_type *descr = get_descriptor<TraitsType>())
{
	if (!ptr)
		return;

	if (descr)
	{
		typename TraitsType::asn_type *asnType = (typename TraitsType::asn_type *)descr;
		ASN_STRUCT_FREE((*asnType), ptr);
	}
	else
		free(ptr);
}

template <typename ToTraitsType, typename FromTraitsType>
static typename ToTraitsType::message_type *_j2735_cast(const typename FromTraitsType::message_type *in)
{
	BOOST_THROW_EXCEPTION(J2735Exception("Bad cast from " +
			battelle::attributes::type_id_name<typename FromTraitsType::message_type>() + " pointer to " +
			battelle::attributes::type_id_name<typename ToTraitsType::message_type>() + " pointer."));
	return 0;
}

template <typename T, typename U>
static T *j2735_cast(const U *in)
{
	T *ret = _j2735_cast< SaeJ2735Traits<T>, SaeJ2735Traits<U> >(in);
	return ret;
}

} /* End namespace j2735 */
} /* End namespace messages */
} /* End namespace tmx */



#endif /* TMX_MESSAGES_SAEJ2735TRAITS_HPP_ */
