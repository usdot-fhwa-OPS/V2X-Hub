
#include "RSUConfigurationList.h"

namespace RSUHealthMonitor
{
    Json::Value RSUConfigurationList::parseJson(std::string &rsuConfigsStr) const
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

    void RSUConfigurationList::parseRSUs(std::string &rsuConfigsStr)
    {
        auto json = parseJson(rsuConfigsStr);
        std::vector<RSUConfiguration> tempConfigs;
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
                auto port = static_cast<uint16_t>(atoi(rsuArray[i][SNMPPortKey].asCString()));
                port != 0 ? config.snmpPort = port : throw RSUConfigurationException("Invalid port number in string format.");
            }
            else
            {
                throw RSUConfigurationException("Either SNMP port does not exist.");
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
                auto _rsuMIBVersionStr = rsuArray[i][RSUMIBVersionKey].asString();
                config.mibVersion = strToMibVersion(_rsuMIBVersionStr);
            }
            else
            {
                throw RSUConfigurationException("RSU mib version does not exist.");
            }
            tempConfigs.push_back(config);
        }
        // Only update RSU configurations when all configs are processed correctly.
        configs.clear();
        configs.assign(tempConfigs.begin(), tempConfigs.end());
    }

    RSUMibVersion RSUConfigurationList::strToMibVersion(std::string &mibVersionStr) const
    {
        boost::trim_left(mibVersionStr);
        boost::trim_right(mibVersionStr);
        // Only support RSU MIB version 4.1
        if (boost::iequals(mibVersionStr, RSU4_1_str))
        {
            return RSUMibVersion::RSUMIB_V_4_1;
        }
        else
        {
            std::stringstream ss;
            ss << "Uknown RSU MIB version: " << mibVersionStr;
            throw RSUConfigurationException(ss.str().c_str());
        }
    }

    std::vector<RSUConfiguration> RSUConfigurationList::getConfigs()
    {
        return configs;
    }

    std::ostream &operator<<(std::ostream &os, const RSUMibVersion &mib)
    {
        const std::vector<std::string> nameMibs = {"UNKOWN MIB",
                                                   "RSU 4.1",
                                                   "NTCIP 1218"};
        return os << nameMibs[static_cast<int>(mib)];
    }

    std::ostream &operator<<(std::ostream &os, const RSUConfiguration &config)
    {
        os << RSUIpKey << ": " << config.rsuIp << ", " << SNMPPortKey << ": " << config.snmpPort << ", " << UserKey << ": " << config.user << ", " << AuthPassPhraseKey << ": " << config.authPassPhrase << ", " << SecurityLevelKey << ": " << config.securityLevel << ", " << RSUMIBVersionKey << ": " << config.mibVersion;
        return os;
    }
}