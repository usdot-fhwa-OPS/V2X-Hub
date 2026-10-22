#pragma once


#include <execinfo.h>
#include <ostream>
#include <string>
#include <stdexcept>
#include <tmx/TmxException.hpp>

namespace tmx::utils {
    /**
 * A generic exception class that maintains a back trace of function calls for later use.
 */
class DbConnectionException: public tmx::TmxException {
public:
	using tmx::TmxException::TmxException;
};
}