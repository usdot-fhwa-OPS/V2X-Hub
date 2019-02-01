/*
 * RTCMTypes.hpp
 *
 *  Created on: Apr 26, 2018
 *      Author: gmb
 */

#ifndef INCLUDE_RTCM_RTCMTYPES_H_
#define INCLUDE_RTCM_RTCMTYPES_H_

#include <algorithm>
#include <bitset>
#include <type_traits>

#include <tmx/messages/byte_stream.hpp>

namespace tmx {
namespace messages {
namespace rtcm {

// Basic number of bits to the integer size needed to store
template <size_t Bits>
struct int_container { typedef typename int_container<Bits - 1>::type type; };

// Template specializations
// Note that for these integer values, at minimum we use 2 bytes since a 1 byte
// integer would get interpreted as a char for printout, which looks strange.
template <> struct int_container<1>  { typedef int16_t type; };
template <> struct int_container<17> { typedef int32_t type; };
template <> struct int_container<33> { typedef int64_t type; };

template <typename C, bool IsSigned>
struct type_chooser {
	typedef typename std::make_signed<typename C::type>::type type;
};

template <typename C>
struct type_chooser<C, false> {
	typedef typename std::make_unsigned<typename C::type>::type type;
};

template <size_t Size, typename T, size_t ByteSize = sizeof(T)>
union integer_data {
	typedef T data_type;

	data_type value;
	tmx::byte_t bytes[ByteSize];

	static constexpr const size_t byteSize = ByteSize;
};

template <size_t Size, typename T>
struct traits {
	typedef std::bitset<Size> bitset;
	typedef T data_type;

	static constexpr const size_t size = Size;
	static constexpr const data_type default_value() { return 0; }
	static const data_type bitmask() { static bitset mask(-1); return (data_type)mask.to_ulong(); }
	static constexpr const size_t byteSize = sizeof(data_type);
	static constexpr const bool is_signed = std::is_signed<T>::value;

	integer_data<Size, T> data;
};

template <size_t Size, typename T = uint>
struct traits_chooser {
	typedef traits<Size,
			typename type_chooser<int_container<Size>, std::is_signed<T>::value>::type> type;
};

template <size_t Size>
struct signed_traits_chooser {
	typedef typename traits_chooser<Size, int>::type type;
};

// This comes from the RTCM 3.3 standard Table 3.3-1, but is applicable to all types
#define def(X, D) typedef traits_chooser<D, X>::type X ## D
#define def10(X, P) \
	def(X, P ## 1); \
	def(X, P ## 2); \
	def(X, P ## 3); \
	def(X, P ## 4); \
	def(X, P ## 5); \
	def(X, P ## 6); \
	def(X, P ## 7); \
	def(X, P ## 8); \
	def(X, P ## 9)
#define def40(X) \
	def10(X, ); \
	def(X, 10); \
	def10(X, 1); \
	def(X, 20); \
	def10(X, 2); \
	def(X, 30); \
	def10(X, 3)
#define defint def40(int)
#define defuint def40(uint)

defint;
defuint;

#undef defuint
#undef defint
#undef def40
#undef def10
#undef def

typedef uint1 bit1;
typedef uint2 bit2;
typedef uint3 bit3;
typedef uint4 bit4;

template <size_t Sz, typename T>
bool checkEnoughBits(T value = 0) {
	static_assert(Sz <= 8 * sizeof(T), "Need a bigger data type to store the data in that number of bytes!");
	return true;
}

template <typename RtcmTraits>
typename RtcmTraits::data_type addSign(typename RtcmTraits::data_type value) {
	// Convert a negative with two's complement
	if (RtcmTraits::is_signed) {
		if (value < 0)
			return (typename RtcmTraits::data_type)(typename RtcmTraits::bitset(value).to_ulong());
		else if ((value >> (RtcmTraits::size - 1)))
			return -1 * ((~value + 1) & RtcmTraits::bitmask());
	}

	return value;
}

template <typename RtcmTraits, typename T>
T rollByte(T byte) {
	std::bitset<RtcmTraits::size> val;
	for (size_t i = 0; i < val.size(); i++) {
		size_t shift = val.size() - i - 1;
		val[i] = (byte & (1 << shift)) == (1 << shift);
	}

	return (T) val.to_ulong();
}

template <typename RtcmTraits, typename Word, typename T = uintmax_t>
T combineWord(Word word, T value) {
	if (value > 0) value <<= RtcmTraits::size;
	value |= (word & RtcmTraits::bitmask());
	return value;
}

template <typename RtcmTraits, typename Word, typename T = uintmax_t>
size_t splitWord(std::vector<Word> &words, T value, size_t pos) {
	words.push_back((Word)((value >> (RtcmTraits::size * pos)) & RtcmTraits::bitmask()));
	return 1;
}

template <typename RtcmTraits, char N>
tmx::byte_t get_Bit(const typename RtcmTraits::data_type &word) {
	static_assert(N <= RtcmTraits::size, "There are not that many bits in the RTCM word.");
	static constexpr char bit = RtcmTraits::size - N;
	return (word >> bit) & 0x01;
}

template <char Bit, char... OtherBits>
struct bit_manipulator {
	template <typename RtcmTraits>
	static tmx::byte_t xor_bits(const typename RtcmTraits::data_type &word) {
		return get_Bit<RtcmTraits, Bit>(word) ^
				bit_manipulator<OtherBits...>::template xor_bits<RtcmTraits>(word);
	}
};

template <char Bit>
struct bit_manipulator<Bit> {
	template <typename RtcmTraits>
	static tmx::byte_t xor_bits(const typename RtcmTraits::data_type &word) {
		return get_Bit<RtcmTraits, Bit>(word);
	}
};

} /* End namespace rtcm */
} /* End namespace messages */
} /* End namespace tmx */


// The attribute builder macros used for these types
#define typesafe_rtcm_getter_builder(C, X, Y, G) \
	typename X::data_type attr_func_name(G, Y)() { \
		return attr_field_name(Y).get(C); \
	} \
	typename X::bitset attr_func_name(G, Y ## Bits)() { \
		return typename X::bitset(attr_func_name(G, Y)()); \
	}
#define typesafe_rtcm_setter_builder(C, X, Y, S, L) \
	void attr_func_name(S, Y)(const typename X::data_type value) { \
		L { \
			attr_field_name(Y).set(C, value); \
		} \
	} \
	void attr_func_name(S, Y ## Bits)(const typename X::bitset bits) { \
		attr_func_name(S, Y)((typename X::data_type)bits.to_ulong()); \
	}
#define rtcm_attribute_builder(T, X, Y, D) \
public: \
	struct Y: public X { \
		static data_type default_value() { return D; } \
	}; \
private: \
	T attr_field_name(Y);
#define ro_rtcm_attribute(C, T, X, Y, G, D) \
	rtcm_attribute_builder(T, X, Y, D) \
public: \
	typesafe_rtcm_getter_builder(C, X, Y, G)
#define rw_rtcm_attribute(C, T, X, Y, G, D, S, L) \
	ro_rtcm_attribute(C, T, X, Y, G, D) \
	typesafe_rtcm_setter_builder(C, X, Y, S, L)
#define std_rtcm_attribute(C, X, Y, D, L) \
	rw_rtcm_attribute(C, battelle::attributes::standard_attribute<Y>, X, Y, get_, D, set_, L)

#endif /* INCLUDE_RTCM_RTCMTYPES_H_ */
