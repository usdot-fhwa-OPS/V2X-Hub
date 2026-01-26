
#include "RSUConfigurationList.h"

namespace RSUHealthMonitor
{
    
    Json::Value RSUConfigurationList::parseJson(const std::string &rsuConfigsStr) const
    {
        JSONCPP_STRING err;
        Json::Value root;
        auto length = static_cast<int>(rsuConfigsStr.length());
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(rsuConfigsStr.c_str(), rsuConfigsStr.c_str() + length, &root, &err))
        {
            std::stringstream ss;
            ss << "Parse RSUs raw string error: " << err;
            throw RSUConfigurationException(ss.str().c_str());
        }
        return root;
    }

    void RSUConfigurationList::parseRSUs(const std::string &rsuConfigsStr)
    {
        auto json = parseJson(rsuConfigsStr);
        std::vector<RSUConfiguration> tempConfigs;
        RSUConfiguration config;
        auto rsuArray = json[RSUSKey];
        if (!rsuArray.isArray())
        {
            throw RSUConfigurationException("RSUConfigurationList: Missing rsuConfigs array.");
        }

        if(rsuArray.empty()){
            return;
        }
        
        for (auto i = 0; i != rsuArray.size(); i++)
        {
            // Parse optional event field
            if (rsuArray[i].isMember(EventKey))
            {
                config.event = rsuArray[i][EventKey].asString();
            }
            else
            {
                config.event = "";
            }

            // Parse RSU configuration (nested object)
            if (rsuArray[i].isMember(RSUKey) && rsuArray[i][RSUKey].isObject())
            {
                const auto &rsuObj = rsuArray[i][RSUKey];
                
                if (rsuObj.isMember(RSUIpKey))
                {
                    config.rsuIp = rsuObj[RSUIpKey].asString();
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: RSU IP [" + std::string(RSUKey) + "." + std::string(RSUIpKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }

                if (rsuObj.isMember(SNMPPortKey))
                {
                    try{
                        config.snmpPort = static_cast<uint16_t>(rsuObj[SNMPPortKey].asUInt());
                    }catch(Json::LogicError & e){
                        auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: Invalid SNMP port number.";
                        throw RSUConfigurationException(errMsg);                        
                    }
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: SNMP port [" + std::string(RSUKey) + "." + std::string(SNMPPortKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }
            }
            else
            {
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: RSU configuration [" + std::string(RSUKey) + "] is required.";
                throw RSUConfigurationException(errMsg);
            }

            // Parse SNMP configuration (nested object)
            if (rsuArray[i].isMember(SNMPKey) && rsuArray[i][SNMPKey].isObject())
            {
                const auto &snmpObj = rsuArray[i][SNMPKey];
                
                if (snmpObj.isMember(UserKey))
                {
                    config.user = snmpObj[UserKey].asString();
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: User [" + std::string(SNMPKey) + "." + std::string(UserKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }

                if (snmpObj.isMember(AuthProtocolKey))
                {
                    config.authProtocol = snmpObj[AuthProtocolKey].asString();
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: Authentication protocol [" + std::string(SNMPKey) + "." + std::string(AuthProtocolKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }

                if (snmpObj.isMember(PrivProtocolKey))
                {
                    config.privProtocol = snmpObj[PrivProtocolKey].asString();
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: Privacy protocol [" + std::string(SNMPKey) + "." + std::string(PrivProtocolKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }
                
                if (snmpObj.isMember(AuthPassPhraseKey))
                {
                    config.authPassPhrase = snmpObj[AuthPassPhraseKey].asString();
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: Authentication pass phrase [" + std::string(SNMPKey) + "." + std::string(AuthPassPhraseKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }
                
                if (snmpObj.isMember(PrivPassPhraseKey))
                {
                    config.privPassPhrase = snmpObj[PrivPassPhraseKey].asString();
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: Privacy pass phrase [" + std::string(SNMPKey) + "." + std::string(PrivPassPhraseKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }

                if (snmpObj.isMember(RSUMIBVersionKey))
                {
                    auto rsuMIBVersionStr = snmpObj[RSUMIBVersionKey].asString();
                    config.mibVersion = tmx::utils::rsu::stringToRSUSpec(rsuMIBVersionStr);
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: RSU MIB version [" + std::string(SNMPKey) + "." + std::string(RSUMIBVersionKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }

                if (snmpObj.isMember(SecurityLevelKey))
                {
                    config.securityLevel = snmpObj[SecurityLevelKey].asString();
                }
                else
                {
                    auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: RSU Security Level [" + std::string(SNMPKey) + "." + std::string(SecurityLevelKey) + "] is required.";
                    throw RSUConfigurationException(errMsg);
                }
            }
            else
            {
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: SNMP configuration [" + std::string(SNMPKey) + "] is required.";
                throw RSUConfigurationException(errMsg);
            }

            tempConfigs.push_back(config);
        }
        // Only update RSU configurations when all configs are processed correctly.
        std::lock_guard<std::mutex> lock(_configMutex);
        configs.clear();
        configs.assign(tempConfigs.begin(), tempConfigs.end());
    }

    void RSUConfigurationList::parseRSUs(tmx::messages::RSURegistrationConfigMessage &msg)
    {
        clearConfigs();
        try
        {
            // Convert TMX RSU configs to internal RSUConfiguration format
            for (const auto &rsuConfig : msg.get_rsuConfigs().rsuConfigs)
            {
                RSUConfiguration config;
                config.rsuIp = rsuConfig.rsu.ip;
                config.snmpPort = rsuConfig.rsu.port;
                config.user = rsuConfig.snmp.user;
                config.authProtocol = rsuConfig.snmp.authProtocol;
                config.privProtocol = rsuConfig.snmp.privacyProtocol;
                config.authPassPhrase = rsuConfig.snmp.authPassPhrase;
                config.privPassPhrase = rsuConfig.snmp.privacyPassPhrase;
                config.securityLevel = rsuConfig.snmp.securityLevel;
                config.mibVersion = tmx::utils::rsu::stringToRSUSpec(rsuConfig.snmp.rsuMIBVersion);
                config.event = rsuConfig.event;                
                addConfig(config);
            }            
        }
        catch (const std::exception &ex)
        {
            throw RSUConfigurationException("Error processing RSU configurations from message: " + std::string(ex.what()));
        }
    }


    std::vector<RSUConfiguration> RSUConfigurationList::getConfigs() const
    {
        std::lock_guard<std::mutex> lock(_configMutex);
        return configs;
    }

    void RSUConfigurationList::addConfig(const RSUConfiguration &config)
    {
        std::lock_guard<std::mutex> lock(_configMutex);
        configs.push_back(config);
    }

    void RSUConfigurationList::clearConfigs()
    {
        std::lock_guard<std::mutex> lock(_configMutex);
        configs.clear();
    }

    std::ostream &operator<<(std::ostream &os, const tmx::utils::rsu::RSU_SPEC &mib)
    {
        
        return os << tmx::utils::rsu::rsuSpecToString(mib);
    }

    std::ostream &operator<<(std::ostream &os, const RSUConfiguration &config)
    {
        os << EventKey << ": " << config.event << ", " 
           << RSUKey << ".{" << RSUIpKey << ": " << config.rsuIp << ", " << SNMPPortKey << ": " << config.snmpPort << "}, "
           << SNMPKey << ".{" << UserKey << ": " << config.user << ", " << AuthProtocolKey << ": " << config.authProtocol 
           << ", " << PrivProtocolKey << ": " << config.privProtocol << ", " << AuthPassPhraseKey << ": " << config.authPassPhrase 
           << ", " << PrivPassPhraseKey << ": " << config.privPassPhrase << ", " << RSUMIBVersionKey << ": " << config.mibVersion 
           << ", " << SecurityLevelKey << ": " << config.securityLevel << "}";
        return os;
    }
}
