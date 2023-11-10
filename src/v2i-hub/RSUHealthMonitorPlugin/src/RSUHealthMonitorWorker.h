#pragma once
#include <string>
#include "RSU_MIB_4_1.h"
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <nmeaparse/nmea.h>
#include "PluginLog.h"

using namespace std;
using namespace tmx::utils;
using namespace tmx::utils::rsu41::mib::oid;

namespace RSUHealthMonitor
{
    enum RSUMibVersion
    {
        UNKOWN_MIB_V = 0,
        RSUMIB_V_4_1 = 1,
        RSUMIB_V_1218 = 2
    };

    struct RSUFieldOIDStruct
    {
        string field;
        string oid;
        bool required; // Indicate whether this field is required to before broadcasting the RSU status.
    };

    /**
     * RSUStatusTable is custome defined RSU status information.
     * The fields are the subset of fields from RSU MIB definition https://github.com/certificationoperatingcouncil/COC_TestSpecs/blob/master/AppNotes/RSU/RSU-MIB.txt
     */
    using RSUStatusConfigTable = vector<RSUFieldOIDStruct>;

    class RSUHealthMonitorWorker
    {
    private:
        // A map of RSU MIB version used and RSUStatusTable
        shared_ptr<map<RSUMibVersion, RSUStatusConfigTable>> _RSUSTATUSConfigMapPtr;

        /**
         * @brief Poupate the RSU status table with the specified version of OIDs and fields.
         * Private: Only allow to initialze the RSU STATUS MAP once
         * @param mibVersion specified
         * @return RSUStatusTable the self defined RSU status https://usdot-carma.atlassian.net/wiki/spaces/WFD2/pages/2640740360/RSU+Health+Monitor+Plugin+Design
         */
        RSUStatusConfigTable constructRsuStatusConfigTable(const RSUMibVersion &mibVersion);

    public:
        // Populate the RSU Status Table with predefined fields and their mapping OIDs in constructor
        RSUHealthMonitorWorker();

        // Access to the RSU status table based in the RSU MIB version provided
        RSUStatusConfigTable GetRSUStatusConfig(const RSUMibVersion &mibVersion);

        /**
         * @brief determine if all required fields in the RSU config map _RSUSTATUSConfigMapPtr present in the input fields
         * Use _RSUSTATUSConfigMapPtr RSU status config map that defines all fields and whether the fields are required.
         * @param RSUMibVersion RSU MIB version
         * @param vector<string> Input RSU fields to verify
         * @return True if all required fields found. Otherwise, false.
         */
        bool isAllRequiredFieldsPresent(const RSUMibVersion &mibVersion, const vector<string> &fields);

        /**
         * @brief Parse NMEA GPS sentense and return GPS related data
         * @param gps_nmea_data NMEA GPS sentense
         * @return map<double, double>  A map of latitude and longitude
         */
        std::map<double, double> ParseRSUGPS(const std::string &gps_nmea_data);

        // Clear the RSU status map in destructor
        ~RSUHealthMonitorWorker();
    };
} // namespace RSUHealthMonitor
