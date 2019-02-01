/*
 * J2735Exception.hpp
 *
 *  Created on: Aug 24, 2017
 *      Author: gmb
 */

#ifndef TMX_MESSAGES_J2735EXCEPTION_HPP_
#define TMX_MESSAGES_J2735EXCEPTION_HPP_

#include <tmx/TmxException.hpp>

namespace tmx
{

/**
 * Exception class for run-time errors that occur in encoding/decoding the J2735 Messages
 */
class J2735Exception: public tmx::TmxException
{
public:
	/**
	 * Create a J2735Exception from another exception.
	 * @param cause The root cause exception
	 */
	J2735Exception(const std::exception &cause): tmx::TmxException(cause) {}

	/**
	 * Create a J2735Exception from a string describing what happened.
	 * @param what The reason for the exception
	 */
	J2735Exception(const std::string &what): tmx::TmxException(what) {}
};


}

#endif /* TMX_MESSAGES_J2735EXCEPTION_HPP_ */
