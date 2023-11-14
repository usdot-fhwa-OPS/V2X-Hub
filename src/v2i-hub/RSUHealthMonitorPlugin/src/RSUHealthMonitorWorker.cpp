#include "RSUHealthMonitorWorker.h"

namespace RSUHealthMonitor
{

    RSUHealthMonitorWorker::RSUHealthMonitorWorker()
    {
        _RSUSTATUSConfigMapPtr = make_shared<map<RSUMibVersion, RSUStatusConfigTable>>();
        // Currently only support RSU MIB version 4.1. Other future supported versions will be inserted here.
        RSUStatusConfigTable rsuRstatusTable = constructRsuStatusConfigTable(RSUMibVersion::RSUMIB_V_4_1);
        _RSUSTATUSConfigMapPtr->insert({RSUMibVersion::RSUMIB_V_4_1, rsuRstatusTable});
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
            PLOG(logERROR) << "Unknown MIB version! " << ex.what();
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

    Json::Value RSUHealthMonitorWorker::getRSUStatus(const RSUMibVersion &mibVersion, const string &_rsuIp, uint16_t &_snmpPort, const string &_securityUser, const string &_authPassPhrase, const string &_securityLevel, long timeout)
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
            PLOG(logINFO) << "Update SNMP client: RSU IP: " << _rsuIp << ", RSU port: " << _snmpPort << ", User: " << _securityUser << ", auth pass phrase: " << _authPassPhrase << ", security level: "
                          << _securityLevel;
            auto _snmpClientPtr = std::make_unique<snmp_client>(_rsuIp, _snmpPort, "", _securityUser, _securityLevel, _authPassPhrase, SNMP_VERSION_3, timeout);

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
                        rsuStatuJson.append(populateJson(config.field, responseVal));
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
}