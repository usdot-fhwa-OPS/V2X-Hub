#include "ERVCloudForwardingWorker.h"

namespace ERVCloudForwardingPlugin
{
    std::string ERVCloudForwardingWorker::constructERVBSMRequest(BsmMessage &msg)
    {
        char xml_str[20000];
        std::string bsmHex = encodeBSMHex(msg);
        auto bsmPtr = msg.get_j2735_data();

        // Check if the BSM is broadcast by the ERV (Emergency Response Vehicle). If not, return empty string.
        if (!IsBSMFromERV(msg))
        {
            return xml_str;
        }

        // If Carma extension does not present, the ERV is not sending the route information, return empty string.
        if (bsmPtr->regional->list.array[0]->regExtValue.present != Reg_BasicSafetyMessage__regExtValue_PR_BasicSafetyMessage_addGrpCarma)
        {
            return xml_str;
        }
        // If there is carma related regional extension value that contains the ERV route points, construct the BSM request with the points.
        auto bsmCarmaRegion = bsmPtr->regional->list.array[0]->regExtValue.choice.BasicSafetyMessage_addGrpCarma;
        std::stringstream route_ss;
        for (int i = 0; i < bsmCarmaRegion.routeDestinationPoints->list.count; i++)
        {
            auto latitude = bsmCarmaRegion.routeDestinationPoints->list.array[i]->lat;
            auto longitude = bsmCarmaRegion.routeDestinationPoints->list.array[i]->Long;
            route_ss << "<point><latitude>" << latitude << "</latitude>"
                     << "<longitude>" << longitude << "</longitude></point>";
        }
        snprintf(xml_str, sizeof(xml_str), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><BSMRequest><id>%s</id><route>%s</route></BSMRequest>", bsmHex.c_str(), route_ss.str().c_str());
        return xml_str;
    }

    std::string ERVCloudForwardingWorker::encodeBSMHex(BsmMessage &msg)
    {
        // Encode the BSM message and return encoded hex string
        tmx::messages::BsmEncodedMessage bsmEncodeMessage;
        tmx::messages::MessageFrameMessage frame_msg(msg.get_j2735_data());
        bsmEncodeMessage.set_data(TmxJ2735EncodedMessage<BasicSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
        return bsmEncodeMessage.get_payload_str();
    }

    bool ERVCloudForwardingWorker::IsBSMFromERV(BsmMessage &msg)
    {
        auto bsm_ptr = msg.get_j2735_data();
        // Check if the BSM contains the regional extesion and PartII. If not, the BSM is not from ERV (Emergency Response Vehicle)
        if (bsm_ptr->regional == NULL || bsm_ptr->partII == NULL)
        {
            return false;
        }
        else
        {
            // The ERV broadcast BSM that has the PartII content, and the specical vehicle extension within the PartII has the emergency response type.
            if (bsm_ptr->partII->list.count > 0 && bsm_ptr->partII->list.array[0]->partII_Value.present == BSMpartIIExtension__partII_Value_PR_SpecialVehicleExtensions && *bsm_ptr->partII->list.array[0]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->responseType == ResponseType_emergency)
            {
                return true;
            }
            return false;
        }
    }

    std::map<long, long> ERVCloudForwardingWorker::ParseGPS(const std::string &gps_nmea_data)
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
            boost::erase_all(longitude_str, ".");
            boost::erase_all(latitude_str, ".");
            result.insert({std::stol(latitude_str), std::stol(longitude_str)});
        }
        catch (nmea::NMEAParseError &e)
        {
            fprintf(stderr, "Error:%s\n", e.message.c_str());
        }
        return result;
    }

    std::string ERVCloudForwardingWorker::constructRSULocationRequest(std::string &rsu_identifier, uint16_t v2xhub_web_port, long latitude, long longitude)
    {
        char xml_str[20000];
        snprintf(xml_str, sizeof(xml_str), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><RSULocationRequest><id>%s</id><latitude>%ld</latitude><longitude>%ld</longitude><v2xhubPort>%d</v2xhubPort></RSULocationRequest>", rsu_identifier.c_str(), latitude, longitude, v2xhub_web_port);
        return xml_str;
    }
} // namespace  ERVCloudForwardingPlugin