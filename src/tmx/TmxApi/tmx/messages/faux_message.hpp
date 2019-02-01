/*
 * faux_message.hpp
 *
 *  Created on: Jun 10, 2016
 *      Author: ivp
 *
 *  Allows tmx::tmx_message derived classes to be used with non c++ 11 compilers.
 *  However, all messaging functionality is stripped out, but the attributes are
 *  still usable as properties.
 */

#ifndef TMX_MESSAGES_FAUX_MESSAGE_HPP_
#define TMX_MESSAGES_FAUX_MESSAGE_HPP_

#include "../attributes/attribute_macros.hpp"

#define std_attribute(C, X, Y, D, L) \
public: \
	struct Y { \
		X value; \
		Y() : value(D) { } \
	}; \
private: \
	Y attr_field_name(Y); \
public: \
	X attr_func_name(get_,Y)() { return attr_field_name(Y).value; } \
	void attr_func_name(set_,Y)(const X value) { attr_field_name(Y).value = value; }

namespace tmx
{
	class tmx_message
	{
	};
	typedef tmx_message message;
}

#endif /* TMX_MESSAGES_FAUX_MESSAGE_HPP_ */
