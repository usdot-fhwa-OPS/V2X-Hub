#pragma once

#include "PluginClient.h"
#include "carma-clock/carma_clock.h"
#include <simulation/SimulationEnvVar.h>
#include <TimeSyncMessage.h>

namespace tmx ::utils {

/**
 * @brief A base plugin class which is can handle simulated time as needed. By setting "SIMULATION_MODE" environment 
 * variavle to "TRUE", this class will add a filter for the TimeSyncMessage. This message is broadcast by the CDASimAdapter 
 * every time a time step happens in simulation. Upon receiving a TimeSyncMessage, the HandleTimeSyncMessage will update
 * the clock to the new simulation time. When "SIMULATION_MODE" is not set to true, the clock will simply return system
 * time using chrono calls.
 */
class PluginClientClockAware : public PluginClient {
public:
    /**
     * @brief Constructor
     * @param name name of plugin
     */
    explicit PluginClientClockAware(const std::string & name);

protected:
    /**
     * @brief Method for child classes to use to retrieve the clock object and get the simulation or real time.
     * @return 
     */
    inline std::shared_ptr<fwha_stol::lib::time::CarmaClock> getClock() const {
        return clock;
    } 

private:
    /**
     * @brief Clock wrapper object that can either store a simulation time or retrieve system time based on initialization
     */
    std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock;

    /**
     * @brief Method to Handle TimeSyncMessage and update clock
     * @param msg TimeSyncMessage broadcast on TMX core 
     * @param routeableMsg 
     */
    void HandleTimeSyncMessage(tmx::messages::TimeSyncMessage &msg, routeable_message &routeableMsg );
};

}