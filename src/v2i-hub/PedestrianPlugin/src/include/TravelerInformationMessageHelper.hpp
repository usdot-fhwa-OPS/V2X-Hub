#pragma once
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <PluginLog.h>
#include <TmxLog.h>
#include <iostream>
#include <regex>

using std::string;
using std::istringstream;
using std::ostringstream;
using tmx::utils::logDEBUG;
using tmx::utils::logERROR;
namespace pt = boost::property_tree;

namespace PedestrianPlugin{
struct TravelerInformationMessageVariables
{
    int16_t msgCnt;
    int startYear;
    int startTime;
    int durationTime;

    // Overload the == operator
    bool operator==(const TravelerInformationMessageVariables& other) const
    {
        return msgCnt == other.msgCnt && startYear == other.startYear && startTime == other.startTime && durationTime == other.durationTime;
    }
};

class TravelerInformationMessageHelper
{
    public:
        
        TravelerInformationMessageHelper() = default;
        // Remove the copy assignment operator and copy constructor
        TravelerInformationMessageHelper(const TravelerInformationMessageHelper&) =delete;
        TravelerInformationMessageHelper& operator=(const TravelerInformationMessageHelper&) = delete;
        //Remove the move assignment operator and move constructor
        TravelerInformationMessageHelper(const TravelerInformationMessageHelper&&) = delete;
        TravelerInformationMessageHelper& operator=(const TravelerInformationMessageHelper&&) = delete;
        ~TravelerInformationMessageHelper() = default;
        /***
         * @brief Converts the XML string to JSON string.
         * @param xml XML string to convert.
         * @return JSON string.
         */
        static string xmlToJson(const string& xml);

        /**
         * @brief Removes special characters from the string.
         * @param str String to remove special characters from.
         * @return String without special characters.
         */
        static string removeSpecialCharacters(const string& str);
        
        /**
         * @brief Converts the JSON string to XML string.
         * @param json JSON string to convert.
         * @return XML string.
         */
        static string jsonToXml(const string& json);
        /**
         * @brief Updates the TravelerInformationMessage(TIM) XML 
         * with new information (msgCnt, startYear, int startTime, durationTime).
         * Return the updated TIM in XML format.
         * @param staticTimXMLIn Static TravelerInformationMessage(TIM) XML.
         * @param msgCount Message count to update.
         * @param startYear Year to update.
         * @param startTime Time to update.
         * @param durationTime Duration time to update.
         * @return Updated TravelerInformationMessage(TIM) XML.
         */
        static string updateTimXML(const string& staticTimXMLIn, const TravelerInformationMessageVariables& timVars);
        /**
         * @brief Updates the TravelerInformationMessage(TIM) tree with new information (msgCnt, startYear, int startTime, durationTime ).
         * @param timTree TravelerInformationMessage(TIM) tree to update.
         */
        static void updateTimTree(pt::ptree &timTree, const TravelerInformationMessageVariables& timVars);
        /**
         * @brief Increases the message count in the TravelerInformationMessage(TIM) XML.
         * @param msgCount Message count to increase.
         * @return Updated message count.
         */
        static int16_t increaseMsgCount(int16_t msgCount);
        /**
         * @brief Calculates the current minute of the year to be included in the Traveler Information Message (TIM).
         */
        static int calculateMinuteOfCurrentYear();
        /***
         * @brief Calculates the current start year to be included in the Traveler Information Message (TIM).
         */
        static int calculateMinuteOfYear(int year, int month, int day, int hour, int minute, int second);
        /**
         * @brief Calculates the current start year to be included in the Traveler Information Message (TIM).
         */ 
        static int calculateCurrentYear();

    };
}
