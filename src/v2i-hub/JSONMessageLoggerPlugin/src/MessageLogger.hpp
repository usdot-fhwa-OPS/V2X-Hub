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
    /**
     * @brief Builds the WARN text for a J2735 message that is not valid ASN.1 (UPER decoding failed).
     * @param msg The message that could not be decoded
     * @param reason Why decoding failed
     * @return For example "Invalid TAM message (ASN.1 UPER decoding failed: ...). Payload: 0025..."
     */
    std::string invalidMessageWarning(tmx::routeable_message& msg, const std::string& reason);
}