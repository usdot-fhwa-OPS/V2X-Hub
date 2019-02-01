/*
 * ISDToJ2735r41.h
 *
 *  Created on: May 31, 2017
 *      Author: gmb
 */

#ifndef INPUTS_ISD_ISDTOJ2735R41_H_
#define INPUTS_ISD_ISDTOJ2735R41_H_

#include "ISDDataAdaptor.hpp"
#include <GeoVector.h>

namespace MapPlugin {

class ISDToJ2735r41 {
public:
	template <typename InputType>
	ISDToJ2735r41(InputType in) {
		tmx::message_container_type container;
		container.load<tmx::JSON>(in);
		adaptor.set_contents(container.get_storage().get_tree());
	}

	virtual ~ISDToJ2735r41();

	MapData *to_map();
	tmx::messages::MapDataMessage to_message();
	tmx::messages::MapDataEncodedMessage to_encoded_message();
private:
	ISDDataAdaptor adaptor;
};

template <typename T>
void allocate(T *&ptr, size_t num = 1)
{
	ptr = (T *)(calloc(num, sizeof(T)));
	if (ptr == NULL)
	{
		BOOST_THROW_EXCEPTION(tmx::TmxException("Unable to allocate memory for " + battelle::attributes::type_name<T>() + ": " + strerror(errno)));
	}
}

template <typename T, typename ValT>
void addValue(T &ref, const ValT value)
{
	std::cout << "Setting " << battelle::attributes::type_name<T>() << " value to " << value << std::endl;
	ref = static_cast<T>(value);
}

template <>
inline void addValue<OCTET_STRING_t, std::string>(OCTET_STRING_t &octetStr, const std::string value)
{
	octetStr.size = value.size();
	allocate(octetStr.buf, value.size());
	memcpy(octetStr.buf, value.data(), value.size());
}

template <size_t N>
inline void addValue(BIT_STRING_t &bitStr, const std::bitset<N> in)
{
	// Encode the bit string in little-endian
	std::string inStr = in.to_string();
	std::bitset<N> bits(std::string(inStr.rbegin(), inStr.rend()));
	constexpr size_t numBytes = bits.size() / 8 + (bits.size() % 8 ? 1 : 0);

	allocate(bitStr.buf, numBytes);
	bitStr.size = (numBytes) * sizeof(uint8_t);
	bitStr.bits_unused = numBytes * 8 - bits.size();

	uint64_t value = bits.to_ulong();
	value = (value << bitStr.bits_unused);
	for (int i = numBytes - 1; i >= 0; i--)
	{
		bitStr.buf[i] |= (value & 0xFF);
		value = (value >> 8);
	}
}

template <typename T, typename ValT>
void addOptionalValue(T *&ptr, const ValT value, const ValT skipVal)
{
	if (value != skipVal)
	{
		allocate(ptr);
		if (ptr) addValue<T, ValT>(*ptr, value);
	}
}

template <size_t N>
inline void addOptionalValue(BIT_STRING_t *&bitStr, const std::bitset<N> bits, const size_t skipVal)
{
	if (bits.to_ullong() != skipVal)
	{
		allocate(bitStr);
		if (bitStr) addValue(*bitStr, bits);
	}
}

template <typename T, typename D>
void decodeEnumFromString(T *&ptr, D &definition, std::string value, const char *tag)
{
	std::stringstream xmlString;
	xmlString << "<" << tag << "><" << value << "/></" << tag << ">";
	xer_decode(0, &definition, (void **)&(ptr), xmlString.str().c_str(), xmlString.str().size());
}

} /* namespace MapPlugin */

#endif /* INPUTS_ISD_ISDTOJ2735R41_H_ */
