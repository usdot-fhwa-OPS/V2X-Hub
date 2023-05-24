#ifndef JSONTOJ2735SSMCONVERTER_H_
#define JSONTOJ2735SSMCONVERTER_H_
#include "jsoncpp/json/json.h"
#include <string>
#include <tmx/j2735_messages/SignalStatusMessage.hpp>
#include <PluginLog.h>

using namespace std;
using namespace tmx::utils;
namespace CARMAStreetsPlugin
{
    class JsonToJ2735SSMConverter
    {
    public:
        // Constructor to initialize object
        JsonToJ2735SSMConverter() = default;
        /***
         * @brief Parse Json string into Json object
         * @param jsonstring that is consumed from a kafka topic
         * @param JsonObject that is populated by the input json string
         * @return boolean. True if parse successfully, false if parse with errors
         */
        bool parseJsonString(const string &consumed_msg, Json::Value &ssmDoc) const;
        /***
         * @brief Read SSM Json document and populate J2735 SSM message
         * @param ssmDoc Json object that contains the SSM information in Json format
         * @param ssmPtr a pointer to J2735 message object. This object will be populated with the values in the Json object
         */
        void toJ2735SSM(const Json::Value &ssmDoc, std::shared_ptr<SignalStatusMessage> ssmPtr) const;
        /***
         * @brief Encode J2735 SSM
         * @param Pointer to  J2735 SSM object
         * @param Encoded J2735 SSM
         */
        void encodeSSM(const std::shared_ptr<SignalStatusMessage> &ssmPtr, tmx::messages::SsmEncodedMessage &encodedSSM) const;

        /***
         * @brief Populate J2735 SignalStatusPackage object with Json Value
         * @param Pointer J2735 SignalStatusPackage object
         * @param Json::Value SignalstatusPackage Json string iterator
         */
        void populateSigStatusPackage(SignalStatusPackage *signalStatusPackage, Json::Value::iterator itr) const;

        ~JsonToJ2735SSMConverter() = default;
    };

}

#endif