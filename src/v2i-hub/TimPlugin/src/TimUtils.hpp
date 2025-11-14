#pragma once

#include <tmx/j2735_messages/TravelerInformationMessage.hpp>
#include <time.h>
#include <PluginLog.h>
#include <filesystem> // Required for std::filesystem
#include <boost/property_tree/xml_parser.hpp>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>

namespace TimPlugin{
	/**
	 * Function to evaluate whether TimMessage is currently active by determining :
	 * 1) Has the start time of the TIM message (UTC) been reached
	 * 2) If duration is not set to indefinite has the duration elapsed
	 * 3) If duration is set to indefinite, no further conditions need to be met.
	 * 
	 * @param std::shared_ptr<tmx::messages::TimMessage> timMsg
	 */
    bool isTimActive(const std::shared_ptr<tmx::messages::TimMessage> &timMsg);
	/**
	 * Helpder function to evaluate whether TimMessage is currently active by determining :
	 * 1) Has the start time of the TIM message (UTC) been reached
	 * 2) If duration is not set to indefinite has the duration elapsed
	 * 3) If duration is set to indefinite, no further conditions need to be met.
	 * 
	 * @param time_t timStartTime (UTC) start time for TIM
	 * @param time_t timStopTime (UTC) calculated by adding TIM start time and duration
	 * @param time_t current time (UTC)
	 * @param long timDuration in minutes used to evaluate whether TIM is intended for indefinite broadcast (indefinite broadcast duration == 32000)
	 */
	bool isTimActive(const time_t &timStartTime, const time_t &timStopTime, const time_t &currentTime, const long timDuration) ; 
	time_t convertTimTime(long year, long minuteOfYear );

	std::shared_ptr<tmx::messages::TimMessage> readTimFile(const std::string &filePath);

	std::shared_ptr<tmx::messages::TimMessage> readTimXml(const std::string &timXml);


    
}