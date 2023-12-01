#include "RSUHealthMonitorWorker.h"
#include <gtest/gtest.h>

namespace RSUHealthMonitor
{
    class test_RSUHealthMonitorWorker : public ::testing::Test
    {
    public:
        std::shared_ptr<RSUHealthMonitorWorker> _rsuWorker = std::make_shared<RSUHealthMonitorWorker>();
    };

    TEST_F(test_RSUHealthMonitorWorker, GetRSUStatusConfig)
    {
        RSUStatusConfigTable rsuStatusConfigTbl = _rsuWorker->GetRSUStatusConfig(RSUMibVersion::RSUMIB_V_4_1);
        ASSERT_EQ(14, rsuStatusConfigTbl.size());

        rsuStatusConfigTbl = _rsuWorker->GetRSUStatusConfig(RSUMibVersion::UNKOWN_MIB_V);
        ASSERT_EQ(0, rsuStatusConfigTbl.size());

        RSUMibVersion mibVersionDefault;
        rsuStatusConfigTbl = _rsuWorker->GetRSUStatusConfig(mibVersionDefault);
        ASSERT_EQ(0, rsuStatusConfigTbl.size());
    }

    TEST_F(test_RSUHealthMonitorWorker, validateAllRequiredFieldsPresent)
    {
        auto config = _rsuWorker->GetRSUStatusConfig(RSUMibVersion::RSUMIB_V_4_1);
        vector<string> requiredFields = {"rsuID", "rsuMibVersion", "rsuFirmwareVersion", "rsuManufacturer", "rsuGpsOutputString", "rsuMode", "rsuChanStatus"};
        ASSERT_TRUE(_rsuWorker->validateAllRequiredFieldsPresent(config, requiredFields));

        requiredFields = {"rsuID", "rsuMibVersion", "rsuFirmwareVersion"};
        ASSERT_FALSE(_rsuWorker->validateAllRequiredFieldsPresent(config, requiredFields));
    }

    TEST_F(test_RSUHealthMonitorWorker, ParseRSUGPS)
    {
        std::string gps_nmea_data = "$GPGGA,142440.00,3857.3065,N,07708.9734,W,2,18,0.65,86.18,M,-34.722,M,,*62";
        auto gps_map = _rsuWorker->ParseRSUGPS(gps_nmea_data);
        ASSERT_EQ(1, gps_map.size());
        double expected_latitude = 38.9551;
        double expected_longitude = -77.1496;
        for (auto itr = gps_map.begin(); itr != gps_map.end(); itr++)
        {
            ASSERT_NEAR(expected_latitude, itr->first, 0.001);
            ASSERT_NEAR(expected_longitude, itr->second, 0.001);
        }
        std::string invalid_gps_nmea_data = "$*GPGGA,invalid";
        auto gps_map_invalid = _rsuWorker->ParseRSUGPS(invalid_gps_nmea_data);
        ASSERT_EQ(0, gps_map_invalid.size());
    }

    TEST_F(test_RSUHealthMonitorWorker, getRSUStatus)
    {
        uint16_t port = 161;
        auto json = _rsuWorker->getRSUStatus(RSUMibVersion::RSUMIB_V_4_1, "127.0.0.1", port, "test", "testtesttest", "authPriv", 1000);
        ASSERT_TRUE(json.empty());

        json = _rsuWorker->getRSUStatus(RSUMibVersion::RSUMIB_V_4_1, "127.0.0.1", port, "test", "test", "authPriv", 1000);
        ASSERT_TRUE(json.empty());

        json = _rsuWorker->getRSUStatus(RSUMibVersion::RSUMIB_V_1218, "127.0.0.1", port, "test", "test", "authPriv", 1000);
        ASSERT_TRUE(json.empty());
    }

    TEST_F(test_RSUHealthMonitorWorker, convertJsonToTMXMsg)
    {
        Json::Value json;
        json["rsuID"] = "RSU4.1";
        json["rsuMode"] = 4;
        auto rsuStatusTmxMsg = _rsuWorker->convertJsonToTMXMsg(json);
        string expectedStr = "{\"rsuID\":\"RSU4.1\",\"rsuMode\":4}\n";
        ASSERT_EQ(expectedStr, rsuStatusTmxMsg.to_string());
    }

    TEST_F(test_RSUHealthMonitorWorker, populateJson)
    {
        Json::Value rsuStatusJson;
        snmp_response_obj stringObj;
        stringObj.type = snmp_response_obj::response_type::STRING;
        std::string gps_nmea_data = "$GPGGA,142440.00,3857.3065,N,07708.9734,W,2,18,0.65,86.18,M,-34.722,M,,*62";
        vector<char> rgps_nmea_data_c;
        copy(gps_nmea_data.begin(), gps_nmea_data.end(), back_inserter(rgps_nmea_data_c));
        stringObj.val_string = rgps_nmea_data_c;

        auto json = _rsuWorker->populateJson("rsuGpsOutputString", stringObj);
        double expected_latitude = 38.9551;
        double expected_longitude = -77.1496;
        ASSERT_NEAR(expected_latitude, json["rsuGpsOutputStringLatitude"].asDouble(), 0.001);
        ASSERT_NEAR(expected_longitude, json["rsuGpsOutputStringLongitude"].asDouble(), 0.001);
        for(const auto& key: json.getMemberNames())
        {
            rsuStatusJson[key] = json[key];
        }

        snmp_response_obj intObj;
        intObj.type = snmp_response_obj::response_type::INTEGER;
        intObj.val_int = 4;

        json = _rsuWorker->populateJson("rsuMode", intObj);
        ASSERT_EQ(4, json["rsuMode"].asInt64());
        rsuStatusJson["rsuMode"] = json["rsuMode"];

        Json::FastWriter fasterWirter;
        string json_str = fasterWirter.write(rsuStatusJson);
        string expectedStr = "{\"rsuGpsOutputString\":\"$GPGGA,142440.00,3857.3065,N,07708.9734,W,2,18,0.65,86.18,M,-34.722,M,,*62\",\"rsuGpsOutputStringLatitude\":38.955108330000002,\"rsuGpsOutputStringLongitude\":-77.149556669999996,\"rsuMode\":4}\n";
        ASSERT_EQ(expectedStr, json_str);
        ASSERT_EQ(4, _rsuWorker->getJsonKeys(rsuStatusJson).size());
        ASSERT_EQ(1, _rsuWorker->getJsonKeys(json).size());
    }

}