#pragma once
#include <unordered_map>
#include <string>
#include <boost/algorithm/string.hpp>
#include <tmx/TmxException.hpp>

using tmx::TmxException;

namespace ODEForwardPlugin
{
    /**
     * @brief Communication mode: UDP or Kafka
     */
    enum class CommunicationMode{
        UDP,
        KAFKA,
        UNSUPPORTED
    };

    class CommunicationModeHelper
    {
    private:
        //Indicator for the communication mode and default value is UNSUPPORTED
        CommunicationMode _currentMode = CommunicationMode::UNSUPPORTED;

        //Map for communication mode to string
        std::unordered_map<CommunicationMode, std::string> _communicationModeMap = {
            {CommunicationMode::UDP, "UDP"},
            {CommunicationMode::KAFKA, "KAFKA"}
        };
    public:
        CommunicationModeHelper()=default;
        ~CommunicationModeHelper()=default;
        /**
         * @brief Compare the communication mode with the given mode
         * @param modeSource The communication mode to be compared
         * @param modeDest The communication mode to be compared with
         * @return true if the communication mode matches, false otherwise
         * @throws TmxException if the communication mode is unknown
         */
        bool compareCommunicationMode(std::string& modeSource, CommunicationMode modeDest);
        /**
         * @brief Set the mode (UDP or KAFKA) of the communication
         * @param modeSource The communication mode to be set
         * @throws TmxException if the communication mode is unknown
         */
        void setMode(std::string& modeSource);
        /**
         * @brief Get the current communication mode
         * @return CommunicationMode enum
         */
        CommunicationMode getCurrentMode() const;
    };
    
}
