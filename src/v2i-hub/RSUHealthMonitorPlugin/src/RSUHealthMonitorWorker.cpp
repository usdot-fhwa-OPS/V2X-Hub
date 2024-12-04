#include "RSUHealthMonitorWorker.h"

namespace RSUHealthMonitor
{

    RSUHealthMonitorWorker::RSUHealthMonitorWorker()
    {
        _RSUSTATUSConfigMapPtr = make_shared<map<RSUMibVersion, RSUStatusConfigTable>>();
        RSUStatusConfigTable rsuRstatusTable_4_1 = constructRsuStatusConfigTable(RSUMibVersion::RSUMIB_V_4_1);
        RSUStatusConfigTable rsuRstatusTable_1218 = constructRsuStatusConfigTable(RSUMibVersion::RSUMIB_V_1218);
        _RSUSTATUSConfigMapPtr->insert({RSUMibVersion::RSUMIB_V_1218, rsuRstatusTable_1218});
        _RSUSTATUSConfigMapPtr->insert({RSUMibVersion::RSUMIB_V_4_1, rsuRstatusTable_4_1});

    }

    RSUStatusConfigTable RSUHealthMonitorWorker::constructRsuStatusConfigTable(const RSUMibVersion &mibVersion) const
    {
        RSUStatusConfigTable rsuStatusTbl;
        // Populate custom defined RSU Status table with RSU MIB version 4.1.
        if (mibVersion == RSUMibVersion::RSUMIB_V_4_1)
        {
            RSUFieldOIDStruct rsuID = {"rsuID", RSU_ID_OID, true};
            rsuStatusTbl.push_back(rsuID);

            RSUFieldOIDStruct rsuMibVersion = {"rsuMibVersion", RSU_MIB_VERSION, true};
            rsuStatusTbl.push_back(rsuMibVersion);

            RSUFieldOIDStruct rsuFirmwareVersion = {"rsuFirmwareVersion", RSU_FIRMWARE_VERSION, true};
            rsuStatusTbl.push_back(rsuFirmwareVersion);

            RSUFieldOIDStruct rsuManufacturer = {"rsuManufacturer", RSU_MANUFACTURER, true};
            rsuStatusTbl.push_back(rsuManufacturer);

            RSUFieldOIDStruct rsuGpsOutputString = {"rsuGpsOutputString", RSU_GPS_OUTPUT_STRING, true};
            rsuStatusTbl.push_back(rsuGpsOutputString);

            RSUFieldOIDStruct rsuIFMIndex = {"rsuIFMIndex", RSU_IFM_INDEX, false};
            rsuStatusTbl.push_back(rsuIFMIndex);

            RSUFieldOIDStruct rsuIFMPsid = {"rsuIFMPsid", RSU_IFM_PSID, false};
            rsuStatusTbl.push_back(rsuIFMPsid);

            RSUFieldOIDStruct rsuIFMDsrcMsgId = {"rsuIFMDsrcMsgId", RSU_IFM_DSRC_MSG_ID, false};
            rsuStatusTbl.push_back(rsuIFMDsrcMsgId);

            RSUFieldOIDStruct rsuIFMTxMode = {"rsuIFMTxMode", RSU_IFM_INDEX, false};
            rsuStatusTbl.push_back(rsuIFMTxMode);

            RSUFieldOIDStruct rsuIFMTxChannel = {"rsuIFMTxChannel", RSU_IFM_TX_CHANNEL, false};
            rsuStatusTbl.push_back(rsuIFMTxChannel);

            RSUFieldOIDStruct rsuIFMEnable = {"rsuIFMEnable", RSU_IFM_ENABLE, false};
            rsuStatusTbl.push_back(rsuIFMEnable);

            RSUFieldOIDStruct rsuIFMStatus = {"rsuIFMStatus", RSU_IFM_STATUS, false};
            rsuStatusTbl.push_back(rsuIFMStatus);

            RSUFieldOIDStruct rsuMode = {"rsuMode", RSU_MODE, true};
            rsuStatusTbl.push_back(rsuMode);

            RSUFieldOIDStruct rsuChanStatus = {"rsuChanStatus", RSU_CHAN_STATUS, true};
            rsuStatusTbl.push_back(rsuChanStatus);
        }
        else if (mibVersion == RSUMibVersion::RSUMIB_V_1218)
        {
            RSUFieldOIDStruct rsuID = {"rsuID", rsuIDOid, true};
            rsuStatusTbl.push_back(rsuID);

            RSUFieldOIDStruct rsuMibVersion = {"rsuMibVersion", rsuMibVersionOid, true};
            rsuStatusTbl.push_back(rsuMibVersion);

            RSUFieldOIDStruct rsuFirmwareVersion = {"rsuFirmwareVersion", rsuFirmwareVersionOid, true};
            rsuStatusTbl.push_back(rsuFirmwareVersion);

            RSUFieldOIDStruct rsuRadioDesc = {"rsuRadioDesc", rsuRadioDescOid, true};
            rsuStatusTbl.push_back(rsuRadioDesc);

            RSUFieldOIDStruct rsuGnssOutputString = {"rsuGnssOutputString", rsuGnssOutputStringOid, true};
            rsuStatusTbl.push_back(rsuGnssOutputString);

            RSUFieldOIDStruct rsuIFMIndex = {"rsuIFMIndex", rsuIFMIndexOid, false};
            rsuStatusTbl.push_back(rsuIFMIndex);

            RSUFieldOIDStruct rsuIFMPsid = {"rsuIFMPsid", rsuIFMPsidOid, false};
            rsuStatusTbl.push_back(rsuIFMPsid);

            RSUFieldOIDStruct rsuIFMTxChannel = {"rsuIFMTxChannel", rsuIFMTxChannelOid, false};
            rsuStatusTbl.push_back(rsuIFMTxChannel);

            RSUFieldOIDStruct rsuIFMEnable = {"rsuIFMEnable", rsuIFMEnableOid, false};
            rsuStatusTbl.push_back(rsuIFMEnable);

            RSUFieldOIDStruct rsuIFMStatus = {"rsuIFMStatus", rsuIFMStatusOid, false};
            rsuStatusTbl.push_back(rsuIFMStatus);

            RSUFieldOIDStruct rsuIFMPriority = {"rsuIFMPriority", rsuIFMPriorityOid, true};
            rsuStatusTbl.push_back(rsuIFMPriority);

            RSUFieldOIDStruct rsuIFMOptions = {"rsuIFMOptions", rsuIFMOptionsOid, true};
            rsuStatusTbl.push_back(rsuIFMOptions);

            RSUFieldOIDStruct rsuIFMPayload = {"rsuIFMPayload", rsuIFMPayloadOid, true};
            rsuStatusTbl.push_back(rsuIFMPayload);

            RSUFieldOIDStruct rsuChanStatus = {"rsuChanStatus", rsuChanStatusOid, true};
            rsuStatusTbl.push_back(rsuChanStatus);

            RSUFieldOIDStruct rsuMode = {"rsuMode", rsuModeOid, true};
            rsuStatusTbl.push_back(rsuMode);
        }
        return rsuStatusTbl;
    }

    bool RSUHealthMonitorWorker::validateAllRequiredFieldsPresent(const RSUHealthMonitor::RSUStatusConfigTable &configTbl, const vector<string> &fields) const
    {
        bool isAllPresent = true;
        for (const auto &config : configTbl)
        {
            if (config.required && find(fields.begin(), fields.end(), config.field) == fields.end())
            {
                isAllPresent = false;
                PLOG(logWARNING) << "No broadcast as required field " << config.field << " is not present!";
            }
        }
        return isAllPresent;
    }

    RSUStatusConfigTable RSUHealthMonitorWorker::GetRSUStatusConfig(const RSUMibVersion &mibVersion) const
    {
        RSUStatusConfigTable result;
        try
        {
            result = _RSUSTATUSConfigMapPtr->at(mibVersion);
        }
        catch (const out_of_range &ex)
        {
            PLOG(logERROR) << "Unknown MIB version! " << mib_version_to_string(mibVersion);
        }
        return result;
    }

    std::map<double, double> RSUHealthMonitorWorker::ParseRSUGPS(const std::string &gps_nmea_data) const
    {
        std::map<double, double> result;
        nmea::NMEAParser parser;
        nmea::GPSService gps(parser);
        try
        {
            parser.readLine(gps_nmea_data);
            std::stringstream ss;
            ss << std::setprecision(8) << std::fixed << gps.fix.latitude << std::endl;
            auto latitude_str = ss.str();
            std::stringstream sss;
            sss << std::setprecision(8) << std::fixed << gps.fix.longitude << std::endl;
            auto longitude_str = sss.str();
            result.insert({std::stod(latitude_str), std::stod(longitude_str)});
            PLOG(logDEBUG) << "Parse GPS NMEA string: " << gps_nmea_data << ". Result (Latitude, Longitude): (" << latitude_str << "," << longitude_str << ")";
        }
        catch (const nmea::NMEAParseError &e)
        {
            PLOG(logERROR) << e.message.c_str();
        }
        return result;
    }

    Json::Value RSUHealthMonitorWorker::getRSUStatus(const RSUMibVersion &mibVersion, const string &_rsuIp, uint16_t &_snmpPort, const string &_securityUser, const std::string &_authProtocol, const std::string &_authPassPhrase, const std::string &_privProtocol, const std::string &_privPassPhrase,const string &_securityLevel, long timeout)
    {
        auto rsuStatusConfigTbl = GetRSUStatusConfig(mibVersion);
        if (rsuStatusConfigTbl.size() == 0)
        {
            PLOG(logERROR) << "RSU status update call failed due to the RSU status config table is empty!";
            return Json::nullValue;
        }
        try
        {
            // Create SNMP client and use SNMP V3 protocol
            PLOG(logINFO) << "SNMP client: RSU IP: " << _rsuIp << ", RSU port: " << _snmpPort << ", User: " << _securityUser << ", Auth protocol: " << _authProtocol << ", Auth pass phrase: " << _authPassPhrase << ", Priv protocol: " << _privProtocol << ", Priv pass phrase: " << _privPassPhrase << ", security level: " << _securityLevel;
            auto _snmpClientPtr = std::make_unique<snmp_client>(_rsuIp, _snmpPort, "public", _securityUser, _securityLevel, _authProtocol, _authPassPhrase, _privProtocol, _privPassPhrase, SNMP_VERSION_3, timeout);

            Json::Value rsuStatuJson;
            // Sending RSU SNMP call for each field as each field has its own OID.
            for (const auto &config : rsuStatusConfigTbl)
            {
                PLOG(logINFO) << "SNMP RSU status call for field:" << config.field << ", OID: " << config.oid;
                snmp_response_obj responseVal;
                if (_snmpClientPtr)
                {
                    auto success = _snmpClientPtr->process_snmp_request(config.oid, request_type::GET, responseVal);
                    if (!success && config.required)
                    {
                        PLOG(logERROR) << "SNMP session stopped as the required field: " << config.field << " failed! Return empty RSU status!";
                        return Json::nullValue;
                    }
                    else if (success)
                    {
                        auto json = populateJson(config.field, responseVal);
                        for(const auto &key: json.getMemberNames())
                        {
                            rsuStatuJson[key] = json[key];
                        }
                    }
                }
            }
            return rsuStatuJson;
        }
        catch (tmx::utils::snmp_client_exception &ex)
        {
            PLOG(logERROR) << ex.what();
            return Json::nullValue;
        }
    }

    Json::Value RSUHealthMonitorWorker::populateJson(const string &field, const snmp_response_obj &response) const
    {
        Json::Value rsuStatuJson;
        if (response.type == snmp_response_obj::response_type::INTEGER)
        {
            rsuStatuJson[field] = response.val_int;
        }
        else if (response.type == snmp_response_obj::response_type::STRING)
        {
            string response_str(response.val_string.begin(), response.val_string.end());
            // Proess GPS nmea string
            if (boost::iequals(field, "rsuGpsOutputString"))
            {
                auto gps = ParseRSUGPS(response_str);
                rsuStatuJson["rsuGpsOutputStringLatitude"] = gps.begin()->first;
                rsuStatuJson["rsuGpsOutputStringLongitude"] = gps.begin()->second;
            }
            rsuStatuJson[field] = response_str;
        }
        return rsuStatuJson;
    }

    RSUStatusMessage RSUHealthMonitorWorker::convertJsonToTMXMsg(const Json::Value &json) const
    {
        Json::FastWriter fasterWirter;
        string json_str = fasterWirter.write(json);
        tmx::messages::RSUStatusMessage rsuStatusMsg;
        rsuStatusMsg.set_contents(json_str);
        return rsuStatusMsg;
    }

    vector<string> RSUHealthMonitorWorker::getJsonKeys(const Json::Value &json) const
    {
        vector<string> keys;
        if (json.isArray())
        {
            for (auto itr = json.begin(); itr != json.end(); itr++)
            {
                if (itr->isObject())
                {
                    for (auto const &field : itr->getMemberNames())
                    {
                        keys.push_back(field);
                    }
                }
            }
        }
        else if (json.isObject())
        {
            for (auto const &field : json.getMemberNames())
            {
                keys.push_back(field);
            }
        }
        return keys;
    }
}