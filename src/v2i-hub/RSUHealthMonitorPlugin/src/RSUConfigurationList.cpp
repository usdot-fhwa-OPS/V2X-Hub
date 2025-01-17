
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
            throw RSUConfigurationException("RSUConfigurationList: Missing RSUS array.");
        }
        for (auto i = 0; i != rsuArray.size(); i++)
        {

            if (rsuArray[i].isMember(RSUIpKey))
            {
                config.rsuIp = rsuArray[i][RSUIpKey].asString();
            }
            else
            {
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: RSU IP [" + std::string(RSUIpKey) + "] is required.";
                throw RSUConfigurationException(errMsg);
            }

            if (rsuArray[i].isMember(SNMPPortKey))
            {
                auto port = static_cast<uint16_t>(atoi(rsuArray[i][SNMPPortKey].asCString()));
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: Invalid SNMP port number in string format.";
                port != 0 ? config.snmpPort = port : throw RSUConfigurationException(errMsg);
            }
            else
            {
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: SNMP port [" + std::string(SNMPPortKey) + "] is required.";
                throw RSUConfigurationException(errMsg);
            }

            if (rsuArray[i].isMember(AuthProtocolKey))
            {
                config.authProtocol = rsuArray[i][AuthProtocolKey].asString();
            }
            else
            {
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: Authentication protocol [" + std::string(AuthProtocolKey) + "] is required.";
                throw RSUConfigurationException(errMsg);
            }

            if (rsuArray[i].isMember(PrivProtocolKey))
            {
                config.privProtocol = rsuArray[i][PrivProtocolKey].asString();
            }
        
            if (rsuArray[i].isMember(AuthPassPhraseKey))
            {
                config.authPassPhrase = rsuArray[i][AuthPassPhraseKey].asString();
            }
            
            if (rsuArray[i].isMember(PrivPassPhraseKey))
            {
                config.privPassPhrase = rsuArray[i][PrivPassPhraseKey].asString();
            }
        

            if (rsuArray[i].isMember(UserKey))
            {
                config.user = rsuArray[i][UserKey].asString();
            }
        
            if (rsuArray[i].isMember(RSUMIBVersionKey))
            {
                auto rsuMIBVersionStr = rsuArray[i][RSUMIBVersionKey].asString();
                config.mibVersion = tmx::utils::rsu::stringToRSUSpec(rsuMIBVersionStr);
            }
            else
            {
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: RSU MIB version [" + std::string(RSUMIBVersionKey) + "] is required.";
                throw RSUConfigurationException(errMsg);
            }

            if (rsuArray[i].isMember(SecurityLevelKey))
            {
                config.securityLevel = rsuArray[i][SecurityLevelKey].asString();
            }
            else
            {
                auto errMsg = "RSUConfigurationList [" + std::to_string(i + 1) + "]: RSU Security Level [" + std::string(SecurityLevelKey) + "] is required.";
                throw RSUConfigurationException(errMsg);
            }
            tempConfigs.push_back(config);
        }
        // Only update RSU configurations when all configs are processed correctly.
        configs.clear();
        configs.assign(tempConfigs.begin(), tempConfigs.end());
    }


    std::vector<RSUConfiguration> RSUConfigurationList::getConfigs() const
    {
        return configs;
    }

    std::ostream &operator<<(std::ostream &os, const tmx::utils::rsu::RSU_SPEC &mib)
    {
        
        return os << tmx::utils::rsu::rsuSpecToString(mib);
    }

    std::ostream &operator<<(std::ostream &os, const RSUConfiguration &config)
    {
        os << RSUIpKey << ": " << config.rsuIp << ", " << SNMPPortKey << ": " << config.snmpPort << ", " << UserKey << ": " << config.user << ", " << AuthProtocolKey << ": " << config.authProtocol << ", " << AuthPassPhraseKey << ": " << config.authPassPhrase << ", " << PrivProtocolKey << ": " << config.privProtocol << ", " << PrivPassPhraseKey << ": " << config.privPassPhrase << ", " << SecurityLevelKey << ": " << config.securityLevel << ", " << RSUMIBVersionKey << ": " << config.mibVersion;
        return os;
    }
}
