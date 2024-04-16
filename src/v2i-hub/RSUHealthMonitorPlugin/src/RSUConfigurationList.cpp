
#include "RSUConfigurationList.h"

namespace RSUHealthMonitor
{
    Json::Value RSUConfigurationList::parseJson(std::string rsuConfigsStr)
    {
        JSONCPP_STRING err;
        Json::Value root;
        auto length = static_cast<int>(rsuConfigsStr.length());
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(rsuConfigsStr.c_str(), rsuConfigsStr.c_str() + length, &root, &err))
        {
            std::stringstream oss;
            oss << "Parse RSUs raw string error: ";
            oss << err.c_str();
            throw RSUConfigurationException(oss.str().c_str());
        }
        return root;
    }
    void RSUConfigurationList::parseRSUs(std::string rsuConfigsStr)
    {
        auto json = parseJson(rsuConfigsStr);
        RSUConfiguration config;
        auto rsuArray = json["RSUS"];
        if (!rsuArray.isArray())
        {
            throw RSUConfigurationException("RSUS is not an array.");
        }
        for (auto i = 0; i != rsuArray.size(); i++)
        {
            if (rsuArray[i].isMember(RSUIpKey))
            {
                config.rsuIp = rsuArray[i][RSUIpKey].asString();
            }
            else
            {
                throw RSUConfigurationException("RSU IP does not exist.");
            }

            if (rsuArray[i].isMember(SNMPPortKey))
            {
                config.snmpPort = atoi(rsuArray[i][SNMPPortKey].asCString());
            }
            else
            {
                throw RSUConfigurationException("SNMP port does not exist.");
            }

            if (rsuArray[i].isMember(AuthPassPhraseKey))
            {
                config.authPassPhrase = rsuArray[i][AuthPassPhraseKey].asString();
            }
            else
            {
                throw RSUConfigurationException("Authentication pass phrase does not exist.");
            }

            if (rsuArray[i].isMember(UserKey))
            {
                config.user = rsuArray[i][UserKey].asString();
            }
            else
            {
                throw RSUConfigurationException("User does not exist.");
            }

            if (rsuArray[i].isMember(RSUMIBVersionKey))
            {
                config.mibVersion = rsuArray[i][RSUMIBVersionKey].asString();
            }
            else
            {
                throw RSUConfigurationException("RSU mib version does not exist.");
            }
            configs.push_back(config);
        }
    }
    std::vector<RSUConfiguration> RSUConfigurationList::getConfigs()
    {
        return configs;
    }
}