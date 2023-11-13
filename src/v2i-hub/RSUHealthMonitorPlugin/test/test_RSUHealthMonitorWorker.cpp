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

    TEST_F(test_RSUHealthMonitorWorker, isAllRequiredFieldsPresent)
    {
        vector<string> requiredFields = {"rsuID", "rsuMibVersion", "rsuFirmwareVersion", "rsuManufacturer", "rsuGpsOutputString", "rsuMode", "rsuChanStatus"};
        ASSERT_TRUE(_rsuWorker->isAllRequiredFieldsPresent(RSUMibVersion::RSUMIB_V_4_1, requiredFields));

        requiredFields = {"rsuID", "rsuMibVersion", "rsuFirmwareVersion"};
        ASSERT_FALSE(_rsuWorker->isAllRequiredFieldsPresent(RSUMibVersion::RSUMIB_V_4_1, requiredFields));
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
}