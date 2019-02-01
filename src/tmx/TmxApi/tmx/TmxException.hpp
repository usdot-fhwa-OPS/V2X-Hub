/*
 * TmxException.h
 *
 *  Created on: Oct 28, 2016
 *      Author: gmb
 */

#ifndef TMX_TMXEXCEPTION_HPP_
#define TMX_TMXEXCEPTION_HPP_

#include <boost/exception/all.hpp>
#include <execinfo.h>
#include <ostream>
#include <string>
#include <stdexcept>
#include <tmx/attributes/type_basics.hpp>

#define BT_BUFFER_SIZE 100

namespace tmx {

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info;

/**
 * A generic exception class that maintains a back trace of function calls for later use.
 */
class TmxException: public std::runtime_error, public virtual boost::exception {
public:
	explicit TmxException(const std::string &what_arg): boost::exception(), std::runtime_error(what_arg)
	{
		SetBacktrace();
	}

	explicit TmxException(const char *what_arg = ""):boost::exception(), std::runtime_error(what_arg)
	{
		SetBacktrace();
	}

	explicit TmxException(const std::exception &err): boost::exception(), std::runtime_error(err.what())
	{
		SetBacktrace();
	}

	explicit TmxException(const boost::exception &err, const std::string &what_arg): boost::exception(err), std::runtime_error(what_arg)
	{
		SetBacktrace();
	}

	explicit TmxException(const boost::exception &err, const char *what_arg = ""): boost::exception(err), std::runtime_error(what_arg)
	{
		SetBacktrace();
	}

	/**
	 * (Re)Set the back trace
	 * @param callerAddr If set, use the pointer for the address of caller.  Useful when
	 * processing invocation from a signal handler.
	 */
	inline void SetBacktrace(void *callerAddr = 0)
	{
		bt.clear();

		std::stringstream ss;

		void *btBuffer[BT_BUFFER_SIZE];
		int nptrs = backtrace(btBuffer, BT_BUFFER_SIZE);

		/* overwrite sigaction with caller's address */
		if (nptrs > 1 && callerAddr > 0)
			btBuffer[1] = callerAddr;

		char **symbols = backtrace_symbols(btBuffer, nptrs);

		/* skip first stack frame (points here) */
		for (int i = 0; symbols != NULL && i < nptrs; i++)
		{
			std::string s(symbols[i]);
			size_t start, end;

			start = s.find_first_of('(');
			if (start >= 0 && start + 1 < s.length())
			{
				start++;
				end = s.find_first_of('+', start);
				if (end >= 0)
				{
					std::string mangledName = s.substr(start, end - start);
					std::string demangledName = battelle::attributes::demangle_type(mangledName.c_str());
					if (demangledName.find("demangling error") == std::string::npos)
					{

						// No error
						ss << s.substr(0, start + 1) << demangledName << s.substr(end) << std::endl;
						continue;
					}
				}
			}

			ss << symbols[i] << std::endl;
		}

		free(symbols);

		bt = ss.str();
	}

	/**
	 * @return The back trace for this exception
	 */
	inline std::string GetBacktrace() const
	{
		return bt;
	}

	/**
	 * A function to write out the exception information, including back trace, to a stream
	 */
	friend inline std::ostream &operator<<(std::ostream &os, const TmxException &ex)
	{
		os << ex.what() << std::endl;
		os << "backtrace: (Hint: Use addr2line -C -e <exe> 0x#######) to find line number)" << std::endl << ex.GetBacktrace();
		os << "diagnostic info:" << std::endl << boost::diagnostic_information(ex);
		return os;
	}
private:
	std::string bt;
};

} /* namespace tmx */

#endif /* TMX_TMXEXCEPTION_HPP_ */
