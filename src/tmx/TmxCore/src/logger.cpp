/*
 * logger.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: gibbsw
 */

#include "logger.h"
#include <sstream>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace dhlogging {

Logger::Logger(std::string fileName)
{
    initialize(fileName);
}

Logger::Logger(Logger const&)
{
}

Logger::~Logger()
{

}

Logger* Logger::logger_ = nullptr;
Logger* Logger::getInstance(std::string logFile)
{
    if ( Logger::logger_ == nullptr ) {
    	logging::add_file_log
		(
			keywords::file_name = logFile,
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
			keywords::format = "%TimeStamp%\t%Message%",
/*
			(
					expr::stream
	                << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
	                << "\t" << logging::trivial::severity
	                << "\t" << expr::smessage
	        ),
*/
			keywords::auto_flush = true
		);

    	logging::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp%\t%Message%");

		logging::core::get()->set_filter
		(
			logging::trivial::severity >= logging::trivial::trace
		);

        logging::add_common_attributes();
        Logger::logger_ = new Logger(logFile);
    }

    return Logger::logger_;
}

void Logger::initialize(std::string fileName)
{
//    BOOST_LOG(log_) << "Hello, World!";
//    BOOST_LOG_SEV(log_, info) << "Hello, World2!";
}

void Logger::logInfo(std::string message)
{

    BOOST_LOG_SEV(log_, info) << message;
}

void Logger::logDebug(std::string message)
{
    BOOST_LOG_SEV(log_, debug) << message;
}

void Logger::logWarn(std::string message)
{
    BOOST_LOG_SEV(log_, warning) << message;
}

void Logger::logError(std::string message)
{
    BOOST_LOG_SEV(log_, error) << message;
}

void Logger::logFatal(std::string message)
{
    BOOST_LOG_SEV(log_, fatal) << message;
}

void Logger::addEventLogEntry(std::string source, std::string description, LogLevel level)
{
	std::stringstream fileLogText;
	fileLogText << "Adding event log";
	if (!source.empty())
	{
		fileLogText << " from '" << source << "'";
	}
	fileLogText << " --> " << description;

	switch(level)
	{
		break;
	case LogLevel_Info:
		LOG_INFO(fileLogText.str());
		break;
	case LogLevel_Warning:
		LOG_WARN(fileLogText.str());
		break;
	case LogLevel_Error:
		LOG_ERROR(fileLogText.str());
		break;
	case LogLevel_Fatal:
		LOG_FATAL(fileLogText.str());
		break;
	case LogLevel_Debug:
	default:
		LOG_DEBUG(fileLogText.str());
		break;
	}

	try {
		LogContext context;
		context.addEventLogEntry(source, description, level);
	} catch (DbException &e) {
		LOG_WARN("MySQL: Unable to add event log entry [" << e.what() << "]");
	}
}

}

/*
int main(int, char*[])
{
    logging::add_common_attributes();

    using namespace logging::trivial;

    dhlogging::Logger::getInstance()->logInfo("himom");

    return 0;
}
*/



