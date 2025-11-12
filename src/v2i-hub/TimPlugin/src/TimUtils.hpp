#pragma once

#include <tmx/j2735_messages/TravelerInformationMessage.hpp>
#include <time.h>
#include <PluginLog.h>
#include <filesystem> // Required for std::filesystem
#include <boost/property_tree/xml_parser.hpp>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>

namespace TimPlugin{
    bool isTimActive(const std::shared_ptr<tmx::messages::TimMessage> &TimMsg);
	bool isTimActive(const time_t &timStartTime, const time_t &timStopTime, const time_t &currentTime, const long timDuration) ; 
	time_t convertTimTime(unsigned int year, long minuteOfYear );

	std::shared_ptr<tmx::messages::TimMessage> readTimFile(const std::string &filePath);

	std::shared_ptr<tmx::messages::TimMessage> readTimXml(const std::string &timXml);


    
}