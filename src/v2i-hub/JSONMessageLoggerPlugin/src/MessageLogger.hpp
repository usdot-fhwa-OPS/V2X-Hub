#pragma once
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_channel_logger.hpp> // For severity channel logger
#include <string>
#include <PluginLog.h>


namespace JSONMessageLoggerPlugin {
    void logRouteableMessage( tmx::routeable_message & msg, boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string>& logger );
    std::string routeableMessageToJsonString(tmx::routeable_message& msg);
}