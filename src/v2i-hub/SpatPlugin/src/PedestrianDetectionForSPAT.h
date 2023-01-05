#pragma once

#include <tmx/j2735_messages/SpatMessage.hpp>

/**
 * A heler class to add pedestrian detection elements to a SPAT message.
*/
class PedestrianDetectionForSPAT
{
public:
    void updateEncodedSpat(tmx::messages::SpatEncodedMessage & spatEncodedMsg,
        std::shared_ptr<tmx::messages::SpatMessage> _spatMessage,
        const std::string & currentPedLanes);
};
