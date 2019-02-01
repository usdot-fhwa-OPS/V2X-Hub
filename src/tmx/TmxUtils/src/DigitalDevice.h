/*
 * DigitalDevice.h
 *
 *  Created on: Nov 10, 2016
 *      Author: gmb
 */

#ifndef SRC_DIGITALDEVICE_H_
#define SRC_DIGITALDEVICE_H_

#include <bitset>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tmx/messages/byte_stream.hpp>

namespace tmx {
namespace utils {

template <unsigned int PinCount>
class DigitalDevice: public std::bitset<PinCount> {
	typedef DigitalDevice<PinCount> thistype;
	typedef std::bitset<PinCount> stdtype;
	typedef uint64_t numtype;
public:
	static constexpr const unsigned int PIN_COUNT = PinCount;

	DigitalDevice(): stdtype() { Clear(); load(); }
	virtual ~DigitalDevice() {}

	// Public API
	virtual void load() {}
	virtual void flush() {}

	virtual void Clear()
	{
		this->reset();
		flush();
	}

	numtype Get()
	{
		return this->to_ulong();
	}

	std::string HexStr()
	{
		char str[NibbleSize()];

		std::string format = "%0";
		format += std::to_string(NibbleSize());
		format += "lX";
		::sprintf((char *)str, format.c_str(), Get());
		return std::string(str, NibbleSize());
	}

	std::string BinStr()
	{
		return this->to_string();
	}

	tmx::byte_stream Bytes()
	{
		return battelle::attributes::attribute_lexical_cast<tmx::byte_stream>(HexStr());
	}

	constexpr size_t Size()
	{
		return PIN_COUNT;
	}

	constexpr size_t NibbleSize()
	{
		return PIN_COUNT / 4 + (PIN_COUNT % 4 ? 1 : 0);
	}

	constexpr size_t ByteSize()
	{
		return PIN_COUNT / 8 + (PIN_COUNT % 8 ? 1 : 0);
	}

	void SetBit(size_t location) { this->set(location); flush(); }
	void ClearBit(size_t location) { this->reset(location); flush(); }
	void FlipBit(size_t location) { this->flip(location); flush(); }

	// Bring forward operators from bitset
	using stdtype::operator[];
	using stdtype::operator==;
	using stdtype::operator!=;
	using stdtype::operator~;
	using stdtype::operator>>;
	using stdtype::operator<<;

	thistype &operator=(const stdtype &other) { this->reset(); this->operator|=(other); return *this; }

	template <typename OtherType>	thistype &operator=(const OtherType &other) { stdtype bits(static_cast<numtype>(other)); return operator=(bits); }
	thistype &operator=(const std::string &value) { return operator=(strtol(value.c_str(), NULL, 0)); }
	thistype &operator=(const char *value) { return operator=(std::string(value)); }
	thistype &operator=(const tmx::byte_stream &value)	{ return operator=(BytesToString(value)); }

	template <typename OtherType>
	bool operator==(OtherType other) { return Get() == static_cast<numtype>(other); }
	bool operator==(const std::string &other) { return operator==(strtol(other.c_str(), NULL, 0)); }
	bool operator==(const char *other) { return operator==(std::string(other)); }
	bool operator==(const tmx::byte_stream &other) { return operator==(BytesToString(other)); }

	template <typename OtherType>
	bool operator!=(OtherType other) { return Get() != static_cast<numtype>(other); }
	bool operator!=(const std::string &other) { return operator!=(strtol(other.c_str(), NULL, 0)); }
	bool operator!=(const char *other) { return operator!=(std::string(other)); }
	bool operator!=(const tmx::byte_stream &other) { return operator!=(BytesToString(other)); }

	thistype &operator&=(const stdtype &other) { stdtype::operator&=(other); flush(); return *this; }

	template <typename OtherType>
	thistype &operator&=(OtherType value) { stdtype bits(static_cast<numtype>(value)); return operator&=(bits); }
	thistype &operator&=(const std::string &value) { return operator&=(strtol(value.c_str(), NULL, 0)); }
	thistype &operator&=(const char *value) { return operator&=(std::string(value)); }
	thistype &operator&=(const tmx::byte_stream &value) { return operator&=(BytesToString(value)); }

	thistype &operator|=(const stdtype &other) { stdtype::operator|=(other); flush(); return *this; }

	template <typename OtherType>
	thistype &operator|=(OtherType value) { stdtype bits(static_cast<numtype>(value)); return operator|=(bits); }
	thistype &operator|=(const std::string &value) { return operator|=(strtol(value.c_str(), NULL, 0)); }
	thistype &operator|=(const char *value) { return operator|=(std::string(value)); }
	thistype &operator|=(const tmx::byte_stream &value) { return operator|=(BytesToString(value)); }

	thistype &operator^=(const stdtype &other) { stdtype::operator^=(other); flush(); return *this; }

	template <typename OtherType>
	thistype &operator^=(OtherType value) { stdtype bits(static_cast<numtype>(value)); return operator^=(bits); }
	thistype &operator^=(const std::string &value) { return operator^=(strtol(value.c_str(), NULL, 0)); }
	thistype &operator^=(const char *value) { return operator^=(std::string(value)); }
	thistype &operator^=(const tmx::byte_stream &value) { return operator^=(BytesToString(value)); }

	thistype &operator>>=(size_t pos) { stdtype::operator>>=(pos); flush(); return *this; }
	thistype &operator<<=(size_t pos) { stdtype::operator<<=(pos); flush(); return *this; }

/*	friend std::ostream &operator<<(std::ostream &os, const DigitalDevice<PinCount> &dev)
	{
		os << dev._data;
		return os;
	}

	friend std::istream &operator>>(std::istream &is, DigitalDevice<PinCount> &dev)
	{
		numtype tmp;
		is >> tmp;
		dev.operator<<(tmp);
		return is;
	}
*/
private:
	std::string BytesToString(const tmx::byte_stream &bytes)
	{
		return std::string("0x")+battelle::attributes::attribute_lexical_cast<std::string>(bytes);
	}
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_DIGITALDEVICE_H_ */
