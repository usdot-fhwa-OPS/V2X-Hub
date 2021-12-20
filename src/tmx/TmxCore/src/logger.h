/*
 * Log.h
 *
 *  Created on: Jul 23, 2014
 *      Author: gibbsw
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#define LOG_SOURCE_CORE "Ivp Core"

/*
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
*/

#include "database/LogContext.h"

#include <map>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <iostream>
#include <sstream>

// Macros for logging
#define  LOG_INFO(f) \
	do { std::stringstream s;\
		s << "INFO\t" << __FILE__ << ", " << __LINE__ << "\t" << f; \
		dhlogging::Logger::getInstance()->logInfo(s.str()); \
	} while (0)

#define  LOG_DEBUG(f) \
	do { std::stringstream s;\
		s << "DEBUG\t" << __FILE__ << ", " << __LINE__ << "\t" << f; \
		dhlogging::Logger::getInstance()->logDebug(s.str()); \
	} while (0)

#define  LOG_WARN(f) \
	do { std::stringstream s;\
		s << "WARNING\t" << __FILE__ << ", " << __LINE__ << "\t" << f; \
		dhlogging::Logger::getInstance()->logWarn(s.str()); \
	} while (0)

#define  LOG_ERROR(f) \
	do { std::stringstream s;\
		s << "ERROR\t" << __FILE__ << ", " << __LINE__ << "\t" << f; \
		dhlogging::Logger::getInstance()->logError(s.str()); \
	} while (0)

#define  LOG_FATAL(f) \
	do { std::stringstream s;\
		s << "FATAL\t" << __FILE__ << ", " << __LINE__ << "\t" << f; \
		dhlogging::Logger::getInstance()->logFatal(s.str()); \
	} while (0)

//#define  LOG_INFO BOOST_LOG_SEV(log_, info)
//BOOST_LOG_SEV(log_, info)
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

using namespace logging::trivial;

namespace dhlogging {
/**
 * Logging manager for the IVP System.
 * \ingroup IVPCore
 */
	class Logger {

	public:
		static Logger* getInstance(std::string logFile = "default.log");

		void logInfo(std::string message);
		void logDebug(std::string message);
		void logWarn(std::string message);
		void logError(std::string message);
		void logFatal(std::string message);

		static void addEventLogEntry(std::string source, std::string description, LogLevel level);

	private:
		explicit Logger(std::string fileName);
		Logger(Logger const&);
		Logger& operator=(Logger const&);
		virtual ~Logger();

		void initialize(std::string fileName);

		src::severity_logger< severity_level > log_;

		static Logger* logger_; // singleton instance
	};
}
#endif /* LOGGER_H_ */
