#include "RSUHealthMonitorPlugin.h"

using namespace RSUHealthMonitor;
using namespace tmx::utils;

namespace RSUHealthMonitor
{

    RSUHealthMonitorPlugin::RSUHealthMonitorPlugin(std::string name) : PluginClient(name)
    {
        UpdateConfigSettings();
        // Send SNMP call to RSU status at configurable interval.
        std::thread rsuStatus_t(&RSUHealthMonitorPlugin::PeriodicRSUStatusReq, this);
        rsuStatus_t.join();
    }

    void RSUHealthMonitorPlugin::UpdateConfigSettings()
    {
        PLOG(logINFO) << "Updating configuration settings.";

        lock_guard<mutex> lock(_configMutex);
        GetConfigValue<uint16_t>("Interval", _interval);
        GetConfigValue<string>("RSUIp", _rsuIp);
        GetConfigValue<uint16_t>("SNMPPort", _snmpPort);
        GetConfigValue<string>("AuthPassPhrase", _authPassPhrase);
        GetConfigValue<string>("SecurityUser", _securityUser);
        GetConfigValue<string>("SecurityLevel", _securityLevel);

        // Update the OID to RSU field mapping
        string rsuOIDMapJsonStr;
        GetConfigValue<string>("RSUOIDConfigMap", rsuOIDMapJsonStr);
        UpdateRSUOIDConfig(rsuOIDMapJsonStr);
    }

    void RSUHealthMonitorPlugin::UpdateRSUOIDConfig(string &json_str)
    {

        if (json_str.length() == 0)
        {
            PLOG(logERROR) << "Error updating RSU OID config due to JSON is empty.";
            return;
        }
        try
        {
            ptree pt;
            istringstream iss(json_str);
            read_json(iss, pt);

            // Clear the RSU OID mapping variable
            _rsuOIDConfigMap.clear();
            BOOST_FOREACH (ptree::value_type &child, pt.get_child("RSUOIDConfig"))
            {
                // Array elements have no names.
                assert(child.first.empty());
                RSUOIDConfig config;
                config.field = child.second.get<string>("RsuField");
                config.oid = child.second.get<string>("OID");
                config.required = child.second.get<bool>("Required");
                // Add RSU OID to the map
                _rsuOIDConfigMap.push_back(config);
            }
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error updating RSU OID config" << e.what();
        }
    }

    void RSUHealthMonitorPlugin::OnConfigChanged(const char *key, const char *value)
    {
        PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }

    void RSUHealthMonitorPlugin::PeriodicRSUStatusReq()
    {
        while (true)
        {
            // Broadcast the RSU status info when there are any RSU responses.
            try
            {
                auto rsuStatusJson = getRSUstatus();

                if (!rsuStatusJson.empty())
                {
                    vector<string> rsuStatusFields;
                    for (auto const &field : rsuStatusJson.getMemberNames())
                    {
                        rsuStatusFields.push_back(field);
                    }
                    // Only broadcast RSU status when all required fields are present.
                    if (isAllRequiredFieldsPresent(rsuStatusFields))
                    {
                        Json::FastWriter fasterWirter;
                        string json_str = fasterWirter.write(rsuStatusJson);
                        tmx::messages::RSUStatusMessage sendRsuStatusMsg;
                        sendRsuStatusMsg.set_contents(json_str);
                        BroadcastMessage(sendRsuStatusMsg);
                        PLOG(logINFO) << "Broadcast RSU status:  " << json_str;
                    }
                }
            }
            catch (std::exception &ex)
            {
                PLOG(logERROR) << ex.what();
            }
            this_thread::sleep_for(chrono::seconds(_interval));
        }
    }

    Json::Value RSUHealthMonitorPlugin::getRSUstatus()
    {
        if (_rsuOIDConfigMap.size() == 0)
        {
            PLOG(logERROR) << "RSU status update call failed due to  RSUOIDConfigMap is empty!";
            return Json::nullValue;
        }
        PLOG(logDEBUG) << "RSU status update call at every " << _interval << " seconds!\n";

        // Create SNMP client and use SNMP V3 protocol
        PLOG(logINFO) << "Update SNMP client: RSU IP: " << _rsuIp << ", RSU port: " << _snmpPort << ", User: " << _securityUser << ", auth pass phrase: " << _authPassPhrase << ", security level: "
                      << _securityLevel;
        auto _snmpClientPtr = std::make_unique<snmp_client>(_rsuIp, _snmpPort, "", _securityUser, _securityLevel, _authPassPhrase, 3, SEC_TO_MICRO);
        if (_snmpClientPtr == nullptr)
        {
            PLOG(logERROR) << "Error creating SNMP client!";
            return Json::nullValue;
        }

        Json::Value rsuStatuJson;
        // Sending RSU SNMP call for each field as each field has its own OID.
        for (auto &config : _rsuOIDConfigMap)
        {
            try
            {
                PLOG(logINFO) << "SNMP RSU status call for field:" << config.field << ", OID: " << config.oid;
                snmp_response_obj responseVal;
                auto success = _snmpClientPtr->process_snmp_request(config.oid, request_type::GET, responseVal);
                if (!success)
                {
                    // If any snmp request failed, stop any furthur snmp requests using the same current snmp session as the next OID will not be created.
                    break;
                }
                else if (success && responseVal.type == snmp_response_obj::response_type::INTEGER)
                {
                    rsuStatuJson[config.field] = responseVal.val_int;
                }
                else if (success && responseVal.type == snmp_response_obj::response_type::STRING)
                {
                    string response_str(responseVal.val_string.begin(), responseVal.val_string.end());
                    PLOG(logDEBUG) << "String value in response: " << response_str;
                    // Proess GPS nmea string
                    if (boost::iequals("rsuGpsOutputString", config.field))
                    {
                        auto gps = ParseGPS(response_str);
                        rsuStatuJson["rsuGpsOutputStringLatitude"] = gps[0];
                        rsuStatuJson["rsuGpsOutputStringLongitude"] = gps[1];
                    }
                    else
                    {
                        rsuStatuJson[config.field] = response_str;
                    }
                }
            }
            catch (std::exception &ex)
            {
                PLOG(logERROR) << "SNMP call failure due to: " << ex.what();
            }
        }
        return rsuStatuJson;
    }

    bool RSUHealthMonitorPlugin::isAllRequiredFieldsPresent(vector<string> fields)
    {
        bool isAllPresent = true;
        for (auto &config : _rsuOIDConfigMap)
        {
            if (config.required && std::find(fields.begin(), fields.end(), config.field) == fields.end())
            {
                isAllPresent = false;
                PLOG(logWARNING) << "No broadcast as required field " << config.field << " is not present!";
            }
        }
        return isAllPresent;
    }

    std::map<long, long> RSUHealthMonitorPlugin::ParseGPS(const std::string &gps_nmea_data)
    {
        std::map<long, long> result;
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
            result.insert({std::stol(latitude_str), std::stol(longitude_str)});
            PLOG(logDEBUG) << "Parse GPS NMEA string: " << gps_nmea_data << ". Result (Latitude, Longitude): (" << latitude_str << "," << longitude_str << ")";
        }
        catch (nmea::NMEAParseError &e)
        {
            fprintf(stderr, "Error:%s\n", e.message.c_str());
        }
        return result;
    }

    RSUHealthMonitorPlugin::~RSUHealthMonitorPlugin()
    {
        _rsuOIDConfigMap.clear();
    }

} // namespace RSUHealthMonitor

int main(int argc, char *argv[])
{
    return run_plugin<RSUHealthMonitorPlugin>("RSU Health Monitor", argc, argv);
}
