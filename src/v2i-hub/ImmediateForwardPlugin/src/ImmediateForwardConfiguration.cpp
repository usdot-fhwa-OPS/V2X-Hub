#include "ImmediateForwardConfiguration.h"

namespace ImmediateForward{
    std::vector<ImfConfiguration> parseImmediateForwardConfiguration(const std::string &config) {
        std::vector<ImfConfiguration> config_obj;
        Json::Reader reader;
        Json::Value root;
        bool parsingSuccessful = reader.parse(config, root);
        if (!parsingSuccessful) {
            throw tmx::TmxException("Error parsing Immediate Forward Configuration: " + 
                reader.getFormattedErrorMessages() );
        }
        if (!root.isArray()) {
            throw tmx::TmxException("Error parsing Immediate Forward Configuration: Root element is not an array!");
        }
        for (Json::Value::ArrayIndex i = 0; i < root.size(); ++i) {
            Json::Value element = root[i];
            config_obj.push_back(parseImfConfiguration(element));
        }
        return config_obj;
    }

    ImfConfiguration parseImfConfiguration(const Json::Value &imfConfig){
        if (!imfConfig.isObject()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: ImfConfig is not an object!");
        }
        ImfConfiguration imfConfiguration;
        imfConfiguration.address = imfConfig[AddressKey].asString();
        imfConfiguration.spec = stringToRSUSpec(imfConfig[RSUSpecKey].asString());
        imfConfiguration.mode = stringToTxMode(imfConfig[TxModeKey].asString());
        imfConfiguration.signMessage = imfConfig[SignKey].asBool();
        if (imfConfiguration.enableHsm) {
            imfConfiguration.enableHsm =imfConfig[EnableHSMKey].asBool();
            imfConfiguration.hsmUrl = imfConfig[HSMURLKey].asString();
        }
        auto messages =  imfConfig[MessagesKey];
        if (!messages.isArray()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: Messages is not an array!");
        }
        for (Json::Value::ArrayIndex i = 0; i < messages.size(); ++i) {
            Json::Value element = messages[i];
            imfConfiguration.messages.push_back(parseMessage(element));
        } 
        return imfConfiguration;
    }

    Message parseMessage( const Json::Value &message) {
        if (!message.isObject()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: Messages element is not an object!");
        }
        Message msg;
        msg.tmxType = message[TmxTypeKey].asString();
        msg.sendType = message[SendTypeKey].asString();
        msg.psid = message[PSIDKey].asString();
        msg.channel = message[ChannelKey].asInt();
        return msg;
    }

    std::string txModeToString(const TxMode &mode) {
        for (auto const &[name, m] : stringToTxModeMap){
                if (mode == m) {
                    return name;
                }
        }
        throw tmx::TmxException("TxMode is not supported!");    
    }

    TxMode stringToTxMode(const std::string &mode) {
        return stringToTxModeMap.at(mode);
    }

    RSU_SPEC stringToRSUSpec(const std::string &spec) {
        return stringToRSUSpecMap.at(spec);
    }

    std::string rsuSpecToString(const RSU_SPEC &spec) {
        for (auto const &[name, m] : stringToRSUSpecMap){
                if (spec == m) {
                    return name;
                }
        }
        throw tmx::TmxException("RSU Specification is not supported!"); 
    }
}