#include "jsoncpp/json/json.h"
#include <memory>
#include <chrono>
#include <string>
#include <iostream>
#include <sstream>
#include <PluginLog.h>
#include <tmx/j2735_messages/SensorDataSharingMessage.hpp>

using namespace tmx::utils;

namespace CARMAStreetsPlugin
{
    class JsonToJ3224SDSMConverter
    {

    public:

        JsonToJ3224SDSMConverter() = default;

        /***
         * @brief Parse Json string into Json object
         * @param jsonstring that is consumed from a kafka topic
         * @param JsonObject that is populated by the input json string
         * @return boolean. True if parse successfully, false if parse with errors
         */
        bool parseJsonString(const std::string &consumed_msg, Json::Value &sdsm_json_output) const;

        /**
         * @brief Convert the Json value with sdsm information info tmx SDSM object.
         * @param json Incoming Json value with sdsm information that is consumed from a Kafka topic.
         * @param sdsm Outgoing J3224 sdsm object that is populated by the json value.
         */

        void convertJson2SDSM(const Json::Value &sdsm_json, const std::shared_ptr<SensorDataSharingMessage_t> &sdsm) const;

        /**
         * @brief Convert the Json value with sdsm information info tmx SDSM object.
         * @param json Incoming Json value with sdsm information that is consumed from a Kafka topic.
         * @param sdsm Outgoing J3224 sdsm object that is populated by the json value.
         */

        void convertJsonToSDSM(const Json::Value &sdsm_json, std::shared_ptr<SensorDataSharingMessage> sdsm) const;
        /***
         * @brief Encode J3224 SDSM
         * @param Pointer to  J3224 SDSM object
         * @param Encoded J3224 SDSM
         */
        void encodeSDSM(const std::shared_ptr<SensorDataSharingMessage_t> &sdsmPtr, tmx::messages::SdsmEncodedMessage &encodedSDSM) const;

        void encodeSDSM(SensorDataSharingMessage_t *sdsmPtr, tmx::messages::SdsmEncodedMessage &encodedSDSM) const;

        ~JsonToJ3224SDSMConverter() = default;
    };

}