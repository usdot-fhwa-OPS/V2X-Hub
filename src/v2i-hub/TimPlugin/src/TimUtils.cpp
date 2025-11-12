#include "TimUtils.hpp"

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
namespace TimPlugin {
    bool isTimActive(const std::shared_ptr<TimMessage> &TimMsg) {
		auto timPtr = TimMsg->get_j2735_data();
		// Start time as minute of the year
		auto startTime = timPtr->dataFrames.list.array[0]->startTime;
		unsigned int startYear = *(timPtr->dataFrames.list.array[0]->startYear);
		if(startTime >= 527040)
		{
			FILE_LOG(logERROR) << "Invalid startTime." << std::endl;
			return false;
		}
		//Duration is unit of minute
		#if SAEJ2735_SPEC < 2020
		auto duration = timPtr->dataFrames.list.array[0]->duratonTime;
		#else 
		auto duration = timPtr->dataFrames.list.array[0]->durationTime; 
		#endif
		
		// Get current UTC Time
		time_t curTimeUTC;
		// Convert start time to t_time
		time_t startTimeUTC = convertTimTime(startYear, startTime);
		time_t stopTimeUTC =  startTimeUTC + (60*duration);

		return isTimActive( startTimeUTC, stopTimeUTC, curTimeUTC, duration);
		
	}
	bool isTimActive(const time_t &timStartTime, const time_t &timStopTime, const time_t &currentTime, const long timDuration) {
		bool isPersist = false;
		if(timDuration >= 32000)
		{
			FILE_LOG(logWARNING) << "TIM duration set to maximum 32000 minutes, indicating TIM is intended for indefinite broadcast.";
			isPersist = true;
		}
		if (timDuration >= 32000) {
			throw TmxException("TIM duration exceeded maximum of 32000 minutes : " + std::to_string(timDuration)  + " minutes!");
		}
		if ( isPersist)  {
			return timStartTime <= currentTime && currentTime <= timStopTime ;
		} 
		else {
			return timStartTime <= currentTime;
		}
		
	}

	time_t convertTimTime(unsigned int year, long minuteOfYear ) {
		// Create tm for start of year
		struct tm tm_utc = {0};
		tm_utc.tm_year = year - 1900; // Years since 1900
		tm_utc.tm_mon = 0;          
		tm_utc.tm_mday = 0;
		tm_utc.tm_hour = 0;
		tm_utc.tm_min = 0;
		tm_utc.tm_sec = 0;
		tm_utc.tm_isdst = 0; 
		// Convert to time T assuming tm is UTC time
		time_t utc_time = timegm(&tm_utc);
		// Add minuteOfYear to utc_time
		return utc_time + minuteOfYear*60; // Convert minute of the year to seconds
	}

	std::shared_ptr<tmx::messages::TimMessage> readTimXml(const std::string &timXml) {
		std::stringstream ss(timXml);
		tmx::message_container_type container;
		container.load<XML>(ss);
		std::shared_ptr<TimMessage> timPtr(new TimMessage(), [](TimMessage *p)
		{
			if (p->get_j2735_data()) {
				ASN_STRUCT_FREE(asn_DEF_TravelerInformation, p->get_j2735_data().get());
			}
		});		
		// auto timPtr = std::make_shared<TimMessage>( [](message_type *p) { });
		timPtr->set_contents(container.get_storage().get_tree());
		return timPtr;
	}

	std::shared_ptr<tmx::messages::TimMessage> readTimFile(const std::string &filePath) {
		if ( std::filesystem::exists(filePath) )  {
			std::ifstream in = std::ifstream(filePath);
			if(in && in.is_open())
			{
				std::stringstream ss;
				ss << in.rdbuf();
				in.close();
				return readTimXml(ss.str());
			}
			else {
				throw TmxException("File " + filePath + " failed to open. System Error : " + strerror(errno) );
			}
		}
		else {
			throw TmxException("File path " + filePath + " does not exist!");
		}
	}

}