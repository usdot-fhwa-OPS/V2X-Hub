#include "include/TravelerInformationMessageHelper.hpp"

namespace PedestrianPlugin{    
    string TravelerInformationMessageHelper::updateTimXML(const std::string& staticTimXMLIn, int msgCount, int startYear, int startTime, int durationTime)
    {
        pt::ptree timTree;
        istringstream iss(staticTimXMLIn);
        pt::read_xml(iss, timTree);
        updateTimTree(timTree, msgCount, startYear, startTime, durationTime);
        std::ostringstream oss;
        pt::write_xml(oss, timTree);
        return oss.str();
    }

    void TravelerInformationMessageHelper::updateTimTree(pt::ptree &timTree, int msgCount, int startYear, int startTime, int durationTime)
    {	
        timTree.put("TravelerInformation.msgCnt", msgCount);
        timTree.put("TravelerInformation.dataFrames.TravelerDataFrame.startYear", startYear);
        timTree.put("TravelerInformation.dataFrames.TravelerDataFrame.startTime", startTime);
        timTree.put("TravelerInformation.dataFrames.TravelerDataFrame.durationTime", durationTime);
    }

    int16_t TravelerInformationMessageHelper::increaseMsgCount(int16_t msgCount)
    {
        if (msgCount >= 127) {
            return 0;
        }
        return msgCount + 1;
    }

    int TravelerInformationMessageHelper::calculateMinuteOfCurrentYear()
    {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        int year = 1900 + ltm->tm_year;
        int month = 1 + ltm->tm_mon;
        int day = ltm->tm_mday;
        int hour = ltm->tm_hour;
        int minute = ltm->tm_min;
        int second = ltm->tm_sec;
        return calculateMinuteOfYear(year, month, day, hour, minute, second);
    }

    int TravelerInformationMessageHelper::calculateMinuteOfYear(int year, int month, int day, int hour, int minute, int second)
    {
        // Set up "current" utc time using values received from FLIR
        std::tm currentTime = {};
        currentTime.tm_year = year - 1900; // Start of all time in this library is 1900. This calculation is specified in the tm library.
        currentTime.tm_mon = month - 1;
        currentTime.tm_mday = day;
        currentTime.tm_hour = hour;
        currentTime.tm_min = minute;
        currentTime.tm_sec = second;

        // Set up start of the "current" year in utc using the year received from FLIR. 
        // Other values set to the beginning of any year, e.g. <Year>-Jan-01 00:00:00.
        std::tm startOfYear = {};
        startOfYear.tm_year = year - 1900;
        startOfYear.tm_mon = 0;
        startOfYear.tm_mday = 1;
        startOfYear.tm_hour = 0;
        startOfYear.tm_min = 0;
        startOfYear.tm_sec = 0;

        try {
            // Calculate difference in seconds. Used to get the current second of the current year.
            std::time_t current = std::mktime(&currentTime);
            std::time_t start = std::mktime(&startOfYear);

            // Convert seconds to minutes. Used to get the current minute of the current year.
            auto secondsDifference = static_cast<int>(std::difftime(current, start));
            int minuteOfYear = secondsDifference / 60;

            return minuteOfYear;
        } catch (const std::runtime_error& e) {
            PLOG(logERROR) << "Error with formatting time. Incorrect values provided. " << e.what();
        }
        return -1;
    }

    int TravelerInformationMessageHelper::calculateCurrentYear()
    {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        return 1900 + ltm->tm_year;
    }
}