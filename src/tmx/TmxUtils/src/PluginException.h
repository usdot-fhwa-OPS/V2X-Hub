/*
 * PluginException.h
 *
 *  Created on: Oct 27, 2016
 *      Author: gmb
 */

#ifndef SRC_PLUGINEXCEPTION_H_
#define SRC_PLUGINEXCEPTION_H_

#include <execinfo.h>
#include <ostream>
#include <string>
#include <stdexcept>
#include <tmx/TmxException.hpp>

namespace tmx {
namespace utils {

/**
 * A generic exception class that maintains a back trace of function calls for later use.
 */
class PluginException: public tmx::TmxException {
public:
	explicit PluginException(const std::string &what_arg): tmx::TmxException(what_arg) {}
	explicit PluginException(const char *what_arg = ""): tmx::TmxException(what_arg) {}
	explicit PluginException(const std::exception &err): tmx::TmxException(err) {}
	virtual ~PluginException() {}
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_PLUGINEXCEPTION_H_ */
