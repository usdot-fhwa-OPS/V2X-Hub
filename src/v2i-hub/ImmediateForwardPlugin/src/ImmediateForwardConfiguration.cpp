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
        for (const auto &element : root) {
            config_obj.push_back(parseImfConfiguration(element));
        }
        return config_obj;
    }

    SNMPAuth parseSNMPAuth(const Json::Value &snmpAuth){
        if (!snmpAuth.isObject()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: SNMPAuth is not an object!");
        }
        SNMPAuth snmpAuthObj;
        snmpAuthObj.user = snmpAuth[UserKey].asString();
        snmpAuthObj.securityLevel = stringToSecurityLevel(snmpAuth[SecurityLevelKey].asString());
        snmpAuthObj.community = snmpAuth[CommunityKey].asString(); 
        if (snmpAuthObj.securityLevel != SecurityLevel::NO_AUTH_NO_PRIV) {
            snmpAuthObj.authProtocol = snmpAuth[AuthProtocolKey].asString();
            snmpAuthObj.authPassPhrase = snmpAuth[AuthPassPhraseKey].asString();
            if (snmpAuthObj.securityLevel ==  SecurityLevel::AUTH_PRIV) {
                snmpAuthObj.privProtocol = snmpAuth[PrivProtocolKey].asString();
                snmpAuthObj.privPassPhrase = snmpAuth[PrivPassPhraseKey].asString();
            }
        }
        return snmpAuthObj; 
    }


    ImfConfiguration parseImfConfiguration(const Json::Value &imfConfig){
        if (!imfConfig.isObject()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: ImfConfig is not an object!");
        }
        ImfConfiguration imfConfiguration;
        imfConfiguration.name = imfConfig[NameKey].asString();
        imfConfiguration.address = imfConfig[AddressKey].asString();
        imfConfiguration.port = imfConfig[PortKey].asUInt();
        imfConfiguration.spec = tmx::utils::rsu::stringToRSUSpec(imfConfig[RSUSpecKey].asString());
        if (imfConfiguration.spec == tmx::utils::rsu::RSU_SPEC::NTCIP_1218) {

            imfConfiguration.snmpAuth = parseSNMPAuth(imfConfig[SNMPAuthKey]);
        }
        imfConfiguration.mode = stringToTxMode(imfConfig[TxModeKey].asString());
        imfConfiguration.signMessage = imfConfig[SignKey].asBool();
        if (imfConfig[EnableHSMKey].isBool()) {
            imfConfiguration.enableHsm = imfConfig[EnableHSMKey].asBool();
            if (imfConfiguration.enableHsm) {
                imfConfiguration.hsmUrl = imfConfig[HSMURLKey].asString();
            }

        }
        auto messages =  imfConfig[MessagesKey];
        if (!messages.isArray()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: Messages is not an array!");
        }
        for (const auto &element : messages) {
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

    std::string securityLevelToString(const SecurityLevel &level) {
        for (auto const &[name, m] : stringToSecurityLevelMap){
                if (level == m) {
                    return name;
                }
        }
        throw tmx::TmxException("SecurityLevel is not supported!");    
    }

    SecurityLevel stringToSecurityLevel(const std::string &level) {
        return stringToSecurityLevelMap.at(level);
    }


}