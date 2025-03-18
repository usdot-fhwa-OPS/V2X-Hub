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
        if(compareCommunicationMode(modeSource, CommunicationMode::KAFKA)){
            _currentMode = CommunicationMode::KAFKA;
        }else if(compareCommunicationMode(modeSource, CommunicationMode::UDP)){
            _currentMode = CommunicationMode::UDP;
        }else{
            _currentMode = CommunicationMode::UNSUPPORTED;
        }
    }

    CommunicationMode CommunicationModeHelper::getCurrentMode() const{
        return _currentMode;
    }
}