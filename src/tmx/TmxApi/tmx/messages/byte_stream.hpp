/*
 * byte_stream.hpp
 *
 *  Created on: Apr 26, 2016
 *      Author: ivp
 */

#ifndef TMX_MESSAGES_BYTE_STREAM_HPP_
#define TMX_MESSAGES_BYTE_STREAM_HPP_

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

#include <tmx/attributes/attribute_cast.hpp>

namespace tmx {

typedef uint8_t byte_t;

typedef std::vector<byte_t> byte_stream;

inline std::ostream &operator<<(std::ostream &os, const tmx::byte_stream &bytes)
{
	char buf[3];
	for(unsigned int i = 0; i < bytes.size(); i++)
	{
		std::snprintf(buf, 3, "%02x", bytes[i]);
		os << buf;
	}

	return os;
}

inline std::istream &operator>>(std::istream &is, tmx::byte_stream &bytes)
{
	std::string str(std::istreambuf_iterator<typename std::istream::char_type>(is), {});

	char buf[3] = { '0', '0', '\0' };
	tmx::byte_t b = 0;
	for (std::string::iterator i = str.begin(); i != str.end(); i++)
	{
		buf[0] = *i;
		i++;
		if (i != str.end())
			buf[1] = *i;
		else
			buf[1] = '0';

		b = (tmx::byte_t)std::strtoul(buf, NULL, 16);
		bytes.push_back(b);
	}

	return is;
}

inline std::string byte_stream_encode(const tmx::byte_stream &bytes)
{
	std::ostringstream oss;
	operator<<(oss, bytes);
	return oss.str();
}

inline tmx::byte_stream byte_stream_decode(const std::string &str)
{
	tmx::byte_stream bytes;
	std::istringstream iss(str);
	operator>>(iss, bytes);
	return bytes;
}

} /* End namespace tmx */

namespace battelle {
namespace attributes {

template <>
inline std::string attribute_lexical_cast<std::string, tmx::byte_stream>(const tmx::byte_stream &bytes)
{
	return tmx::byte_stream_encode(bytes);
}

template <>
inline tmx::byte_stream attribute_lexical_cast<tmx::byte_stream, std::string>(const std::string &str)
{
	return tmx::byte_stream_decode(str);
}

} /* End namespace attributes */
} /* End namespace battelle */

#endif /* TMX_MESSAGES_BYTE_STREAM_HPP_ */
