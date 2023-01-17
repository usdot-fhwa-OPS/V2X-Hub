#include "PedestrianDetectionForSPAT.h"

#include <vector>

using namespace tmx::messages;

void PedestrianDetectionForSPAT::updateEncodedSpat(SpatEncodedMessage & spatEncodedMsg,
        std::shared_ptr<SpatMessage> _spatMessage,
        const std::string & currentPedLanes)  const
{
    // Add pedestrian lanes with active detections and clear the rest
    auto spat = _spatMessage->get_j2735_data();
    // check for valid SPAT and also if ped lanes is filled in
    if (spat && spat->intersections.list.array && spat->intersections.list.count > 0 && currentPedLanes.length()) {
        // parse ped zone string into set of int values
        std::vector<LaneConnectionID_t> zones;
        {
            std::vector<char> zoneList(currentPedLanes.length());
            std::copy(currentPedLanes.begin(), currentPedLanes.end(), zoneList.begin());
            char *restOfString = nullptr;
            auto c = strtok_r(zoneList.data(), ",", &restOfString);

            while (c != nullptr) {
                zones.push_back(strtol(c, nullptr, 0));
                c = strtok_r(nullptr, ",", &restOfString);
            }
        }

        if (!zones.empty()) {
            ManeuverAssistList *&mas = spat->intersections.list.array[0]->maneuverAssistList;
            mas = (ManeuverAssistList *) calloc(1, sizeof(ManeuverAssistList));
            std::sort(zones.begin(), zones.end());
            // add a connection maneuver for each ped zone and set the ped detect flag
            std::for_each(zones.begin(), zones.end(), [&mas](LaneConnectionID_t connectionId)
                {
                    auto connectionManManeuverAssist = (ConnectionManeuverAssist *) calloc(1, sizeof(ConnectionManeuverAssist));
                    connectionManManeuverAssist->connectionID = connectionId;
                    connectionManManeuverAssist->pedBicycleDetect = (PedestrianBicycleDetect_t *) calloc(1, sizeof(PedestrianBicycleDetect_t));
                    *(connectionManManeuverAssist->pedBicycleDetect) = 1;
                    ASN_SEQUENCE_ADD(mas, connectionManManeuverAssist);
                }
            );
        }
    }

    MessageFrameMessage frame(_spatMessage->get_j2735_data());
    spatEncodedMsg.set_data(TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame));
    //Free the memory allocated for MessageFrame
    free(frame.get_j2735_data().get());
}
