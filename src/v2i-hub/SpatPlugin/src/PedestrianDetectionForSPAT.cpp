#include "PedestrianDetectionForSPAT.h"

#include <vector>

using namespace tmx::messages;

void PedestrianDetectionForSPAT::updateEncodedSpat(SpatEncodedMessage & spatEncodedMsg,
        std::shared_ptr<SpatMessage> _spatMessage,
        const std::string & currentPedLanes)
{
    // Add pedestrian lanes with active detections and clear the rest
    auto spat = _spatMessage->get_j2735_data();
    if (spat && spat->intersections.list.array && spat->intersections.list.count > 0) {
        char *zoneList = strdup(currentPedLanes.c_str());
        std::vector<LaneConnectionID_t> zones;
        char *restOfString = nullptr;
        auto c = strtok_r(zoneList, ",", &restOfString);

        while (c != nullptr) {
            zones.push_back(strtol(c, nullptr, 0));
            c = strtok_r(nullptr, ",", &restOfString);
        };

        free(zoneList);
        zoneList = nullptr;
        c = nullptr;

        if (!zones.empty()) {
            ManeuverAssistList *&mas = spat->intersections.list.array[0]->maneuverAssistList;
            mas = (ManeuverAssistList *) calloc(1, sizeof(ManeuverAssistList));
            std::sort(zones.begin(), zones.end());
            for (size_t i = 0; i < zones.size(); i++) {
                auto connectionManManeuverAssist = (ConnectionManeuverAssist *) calloc(1, sizeof(ConnectionManeuverAssist));
                connectionManManeuverAssist->connectionID = zones[i];
                connectionManManeuverAssist->pedBicycleDetect = (PedestrianBicycleDetect_t *) calloc(1, sizeof(PedestrianBicycleDetect_t));
                *(connectionManManeuverAssist->pedBicycleDetect) = 1;
                ASN_SEQUENCE_ADD(mas, connectionManManeuverAssist);
            }
        }
    }

    MessageFrameMessage frame(_spatMessage->get_j2735_data());
    spatEncodedMsg.set_data(TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame));
    //Free the memory allocated for MessageFrame
    free(frame.get_j2735_data().get());
}
