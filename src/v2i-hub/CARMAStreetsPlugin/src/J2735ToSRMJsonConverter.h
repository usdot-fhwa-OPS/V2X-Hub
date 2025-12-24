#include "jsoncpp/json/json.h"
#include <tmx/j2735_messages/SignalRequestMessage.hpp>

namespace CARMAStreetsPlugin
{
    class J2735ToSRMJsonConverter
    {
    private:
        const std::string MsgType = "SRM";
        const float FIFTY_TH = 0.02;
        const float DEGREE_TO_TENTH_MICRODEGREE = 10000000.0; 
        const int TEN_CM_TO_METER = 10;

    public:
        /***
         *@brief Constructor to initialize the SRM to JSON converter
        */
        J2735ToSRMJsonConverter();
        /***
         * @brief Convert J2735 SRM message into SignalRequests in JSON format
         * @param Vector of JSON to be populated with SignalRequests from J2735
         * @param SrmMessage that has the J2735 SRM information
        */
        void toSRMJsonV(std::vector<Json::Value> &jsonV, tmx::messages::SrmMessage *srm);
        ~J2735ToSRMJsonConverter();
    };
}