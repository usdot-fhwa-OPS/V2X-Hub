/*
 * SaeJ2735Traits.hpp
 *
 *  Created on: Aug 18, 2017
 *      Author: gmb
 */

#pragma once

#include <tmx/attributes/type_basics.hpp>
#include <tmx/messages/J2735Exception.hpp>
#include <type_traits>
#include <utility>

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

// Primary template: returns false for everything else
// Helper for is_instance_of_v
template <typename T, template <typename...> class Template>
struct is_instance_of : std::false_type {};

// Partial specialization: matches when T is an instantiation of Template
// Helper for is_instance_of_v
template <template <typename...> class Template, typename... Args>
struct is_instance_of<Template<Args...>, Template> : std::true_type {};

/**
 * Used to determine whether type is instance of template class.
 * std::is_same_v only works with full defined classes.
 */
template <typename T, template <typename...> class Template>
inline constexpr bool is_instance_of_v = is_instance_of<T, Template>::value;



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
/**
 * A helper function to allocate a zeroed ASN.1 struct and manage it with shared_ptr and a custom deleter.
 * @return A shared_ptr to the allocated ASN.1 struct
 * @tparam TraitsType The traits type for the ASN.1 struct to allocate
 */
template <typename TraitsType>
static std::shared_ptr<typename TraitsType::message_type> j2735_create() {
	std::shared_ptr<typename TraitsType::message_type> alloc(
		static_cast<typename TraitsType::message_type *>(calloc(1, sizeof(typename TraitsType::message_type))),
		[](typename TraitsType::message_type *rawPtr) {
			j2735_destroy<TraitsType>(rawPtr);
		}
	);
	return alloc;
}
/**
 * A helper function to allocate a zeroed ASN.1 struct.
 * @tparam T The ASN.1 struct type to allocate
 * @return A pointer to the allocated ASN.1 struct
 * @note The allocated struct should not be directly freed, rather the large ASN.1 C struct created by j2735_create() will recursively free all of its members, including this struct
 */
template <typename T>
T *AllocAsn() {
	return static_cast<T *>(calloc(1, sizeof(T)));
}

/**
 * A helper function to allocate a zeroed ASN.1 struct and initialize it with a value.
 * @tparam T The ASN.1 struct type to allocate
 * @param value The value to initialize the allocated struct with
 * @return A pointer to the allocated ASN.1 struct
 * @note The allocated struct should not be directly freed, rather the large ASN.1 C struct created by j2735_create() will recursively free all of its members, including this struct
 */
template <typename T>
T *AllocAsn(T value) {
	auto *p = AllocAsn<T>();
	*p = value;
	return p;
}

/**
 * A helper function to allocate a zeroed ASN.1 buffer.
 * @param size The size of the buffer to allocate
 * @return A pointer to the allocated buffer
 * @note The allocated buffer should not be directly freed, rather the large ASN.1 C struct created by j2735_create() will recursively free all of its members, including this struct
 */
inline uint8_t *AllocAsnBuffer(size_t size) {
	return static_cast<uint8_t *>(calloc(size, 1));
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



