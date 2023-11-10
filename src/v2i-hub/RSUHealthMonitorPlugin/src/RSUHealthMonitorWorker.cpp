#include "RSUHealthMonitorWorker.h"

namespace RSUHealthMonitor
{

    RSUHealthMonitorWorker::RSUHealthMonitorWorker()
    {
        _RSUSTATUSConfigMapPtr = make_shared<map<RSUMibVersion, RSUStatusConfigTable>>();
        // Current only support RSU MIB version 4.1. Other future supported versions will be inserted here.
        RSUStatusConfigTable rsuRstatusTable = constructRsuStatusConfigTable(RSUMIB_4_1);
        _RSUSTATUSConfigMapPtr->insert({RSUMIB_4_1, rsuRstatusTable});
    }

    RSUStatusConfigTable RSUHealthMonitorWorker::constructRsuStatusConfigTable(const RSUMibVersion &mibVersion)
    {
        RSUStatusConfigTable rsuStatusTbl;
        // Populate custome defined RSU Status table with RSU MIB version 4.1.
        if (mibVersion == RSUMIB_4_1)
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

    bool RSUHealthMonitorWorker::isAllRequiredFieldsPresent(const RSUMibVersion &mibVersion, const vector<string> &fields)
    {
        bool isAllPresent = true;
        for (auto &config : GetRSUStatusConfig(mibVersion))
        {
            if (config.required && find(fields.begin(), fields.end(), config.field) == fields.end())
            {
                isAllPresent = false;
                PLOG(logWARNING) << "No broadcast as required field " << config.field << " is not present!";
            }
        }
        return isAllPresent;
    }

    RSUStatusConfigTable RSUHealthMonitorWorker::GetRSUStatusConfig(const RSUMibVersion& mibVersion)
    {
        return _RSUSTATUSConfigMapPtr->at(mibVersion);
    }

    std::map<double, double> RSUHealthMonitorWorker::ParseRSUGPS(const std::string &gps_nmea_data)
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
        catch (nmea::NMEAParseError &e)
        {
            fprintf(stderr, "Error:%s\n", e.message.c_str());
        }
        return result;
    }

    RSUHealthMonitorWorker::~RSUHealthMonitorWorker()
    {
        _RSUSTATUSConfigMapPtr->clear();
    }
}