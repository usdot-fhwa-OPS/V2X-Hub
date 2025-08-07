/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
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
        if (snmpAuth[SnmpTimeoutKey].isUInt()) {
            snmpAuthObj.snmpTimeout = snmpAuth[SnmpTimeoutKey].asUInt();
        }
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
        if (imfConfig[PayloadPlaceholderKey].isString()) {
            imfConfiguration.payloadPlaceholder = imfConfig[PayloadPlaceholderKey].asString();
        }
    
        const auto &messageConfigs =  imfConfig[MessagesKey];
        if (!messageConfigs.isArray()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: Messages is not an array!");
        }
        for (const auto &element : messageConfigs) {
            imfConfiguration.messageConfigs.push_back(parseMessage(element));
        }
        return imfConfiguration;
    }

    MessageConfig parseMessage( const Json::Value &message) {
        if (!message.isObject()) {
            throw tmx::TmxException("Error parsing Immediate Forward configuration: Messages element is not an object!");
        }
        MessageConfig msg;
        msg.tmxType = message[TmxTypeKey].asString();
        msg.sendType = message[SendTypeKey].asString();
        msg.psid = message[PSIDKey].asString();
        if (message[ChannelKey].isInt()) {
            msg.channel = message[ChannelKey].asInt();
        }
        else {
            // Default value (see routeable_message.h addDsrcMetadata())
            msg.channel = 183;
        }
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
        try {
            return stringToTxModeMap.at(mode);
        } 
        catch (const std::out_of_range& ) {
            throw tmx::TmxException("TxMode " + mode + " is not supported!");    
        }
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
        try{
            return stringToSecurityLevelMap.at(level);
        }
        catch (const std::out_of_range& ) {
            throw tmx::TmxException("SecurityLevel " + level + " is not supported!");    
        }
    }


}