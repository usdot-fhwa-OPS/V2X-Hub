#include "include/TravelerInformationMessageHelper.hpp"

namespace PedestrianPlugin{  
    string TravelerInformationMessageHelper::xmlToJson(const string& xml){
        pt::ptree timTree;
        istringstream iss(xml);
        pt::read_xml(iss, timTree);
        std::ostringstream oss;
        pt::write_json(oss, timTree);
        auto output = removeSpecialCharacters(oss.str());
        return output;
    }  

    string TravelerInformationMessageHelper::jsonToXml(const string& json){
        pt::ptree timTree;
        istringstream iss(json);
        pt::read_json(iss, timTree);
        std::ostringstream oss;
        pt::write_xml(oss, timTree);
        auto output = removeSpecialCharacters(oss.str());
        return output;
    }

    string  TravelerInformationMessageHelper::removeSpecialCharacters(const string& str){
        string output = std::regex_replace(str, std::regex(R"(\n)"), "");
        output = std::regex_replace(output, std::regex(R"(\s+)"), "");
        output = std::regex_replace(output, std::regex(R"(<\?xml\s*version=\"1\.0\"\s*encoding=\"utf-8\"\?>)"), "");
        return output;
    }

    string TravelerInformationMessageHelper::updateTimXML(const std::string& staticTimXMLIn, const TravelerInformationMessageVariables& timVars)
    {
        pt::ptree timTree;
        istringstream iss(staticTimXMLIn);
        pt::read_xml(iss, timTree);
        updateTimTree(timTree, timVars);
        std::ostringstream oss;
        pt::write_xml(oss, timTree);
        return oss.str();
    }

    void TravelerInformationMessageHelper::updateTimTree(pt::ptree &timTree, const TravelerInformationMessageVariables& timVars)
    {	
        timTree.put("TravelerInformation.msgCnt", timVars.msgCnt);
        timTree.put("TravelerInformation.dataFrames.TravelerDataFrame.startYear", timVars.startYear);
        timTree.put("TravelerInformation.dataFrames.TravelerDataFrame.startTime", timVars.startTime);
        timTree.put("TravelerInformation.dataFrames.TravelerDataFrame.durationTime", timVars.durationTime);
    }

    void  TravelerInformationMessageHelper::updateTimDescNameWithTimestamp(pt::ptree &timTree){
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        auto& geographs = timTree.get_child("TravelerInformation.dataFrames.TravelerDataFrame.regions");
        for (auto& frame : geographs) {
            //Replace GeographicalPath descriptive name with unique timestamp
            frame.second.put("name", std::to_string(now));
        }
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
        time_t now = time(nullptr);
        tm ltm ;
        localtime_r(&now, &ltm);
        int year = 1900 + ltm.tm_year;
        int month = 1 + ltm.tm_mon;
        int day = ltm.tm_mday;
        int hour = ltm.tm_hour;
        int minute = ltm.tm_min;
        int second = ltm.tm_sec;
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
        time_t now = time(nullptr);
        tm ltm ;
        localtime_r(&now, &ltm);
        return 1900 + ltm.tm_year;
    }
}