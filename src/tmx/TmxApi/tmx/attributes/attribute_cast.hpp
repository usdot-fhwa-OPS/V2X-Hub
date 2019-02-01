/*
 * @file attribute_cast.hpp
 *
 *  Created on: Apr 14, 2016
 *      Author: ivp
 */

#ifndef TMX_ATTRIBUTES_ATTRIBUTE_CAST_HPP_
#define TMX_ATTRIBUTES_ATTRIBUTE_CAST_HPP_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/lexical_cast.hpp>
#pragma GCC diagnostic pop
#include <type_traits>

namespace battelle { namespace attributes {
/**
 * Template that helps cast one type to another using Boost's lexical cast.  The default case
 * assumes that there exists a lexical cast already.
 */
template <bool toEnumType, bool fromEnumType>
struct cast_helper {
	template <typename ToType, typename FromType>
	ToType cast(const FromType &val) {
		return boost::lexical_cast<ToType>(val);
	}
};

/**
 * Specialization that handles casting from any enum type to some other type like a string
 * using Boost's lexical cast.
 */
template <>
struct cast_helper<false, true> {
	template <typename ToType, typename FromType>
	ToType cast(const FromType &val) {
		const int *x = reinterpret_cast<const int *>(&val);
		return boost::lexical_cast<ToType>(*x);
	}
};

/**
 * Specialization that handles casting from some type like a string to any enum type using
 * Boost's lexical cast.
 */
template <>
struct cast_helper<true, false> {
	template <typename ToType, typename FromType>
	ToType cast(const FromType &val) {
		const int x = boost::lexical_cast<int>(val);
		return static_cast<ToType>(x);
	}
};

/**
 * Specialization that handles casting from one enum type to another.  Direct
 * static casting is used in this case.
 */
template <>
struct cast_helper<true, true> {
	template <typename ToType, typename FromType>
	ToType cast(const FromType &val) {
		const int x = static_cast<int>(val);
		return static_cast<ToType>(x);
	}
};

/**
 * This function casts the supplied data to different attribute types
 * using Boost's lexical cast.
 *
 * @param val The value to cast
 * @return A re-interpreted version of the same value with a different type.
 */
template <typename ToType, typename FromType>
ToType attribute_lexical_cast(const FromType &val) {
	constexpr bool tEnumType = std::is_enum<ToType>::value;
	constexpr bool fEnumType = std::is_enum<FromType>::value;

	cast_helper<tEnumType, fEnumType> helper;
	return helper.template cast<ToType>(val);
}

}} /* End namespace */

#endif /* TMX_ATTRIBUTES_ATTRIBUTE_CAST_HPP_ */
