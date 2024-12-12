#include "CommunicationModeHelper.h"

namespace ODEForwardPlugin{

    bool CommunicationModeHelper::compareCommunicationMode(std::string& modeSource, CommunicationMode modeDest){
        boost::algorithm::trim(modeSource);
        boost::algorithm::to_upper(modeSource);
        if(boost::iequals(modeSource, _communicationModeMap.at(modeDest))){
            return true;
        }
        return false;
    }

    void CommunicationModeHelper::setMode(std::string& modeSource){
        _isKAFKAMode = compareCommunicationMode(modeSource, CommunicationMode::KAFKA);
        _isUDPMode = compareCommunicationMode(modeSource, CommunicationMode::UDP);
    }

    bool CommunicationModeHelper::getKafkaMode() const{
        return _isKAFKAMode;
    }

    bool CommunicationModeHelper::getUDPMode() const{
        return _isUDPMode;
    }
}