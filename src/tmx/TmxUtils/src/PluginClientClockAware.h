#pragma once

#include "PluginClient.h"
#include "/opt/carma/include/carma-clock/carma_clock.h"
#include <simulation/SimulationEnvUtils.h>
#include <TimeSyncMessage.h>
#include "Clock.h"

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
     * @brief Constructor. Plugins that extent this constructor must at some point call SubcribeToMessages in 
     * constructor to register TimeMessageHandler.
     * @param name name of plugin
     */
    explicit PluginClientClockAware(const std::string & name);
    /**
     * @brief Method to Handle TimeSyncMessage and update clock
     * @param msg TimeSyncMessage broadcast on TMX core 
     * @param routeableMsg 
     */
    virtual void HandleTimeSyncMessage(tmx::messages::TimeSyncMessage &msg, routeable_message &routeableMsg );

    
protected:
    /**
     * @brief Method for child classes to use to retrieve the clock object and get the simulation or real time.
     * @return 
     */
    inline std::shared_ptr<fwha_stol::lib::time::CarmaClock> getClock() const {
        return clock;
    }

    void OnStateChange(IvpPluginState state) override; 

    bool isSimulationMode() const;

    
private:
    /**
     * @brief Clock wrapper object that can either store a simulation time or retrieve system time based on initialization
     */
    std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock;
    /**
	 * @brief Status label simulation time to be displayed by each plugin.
	 */
	const char* Key_Simulation_Time_Step = "Simulation Time Step ";
    /**
	 * @brief Status label to indicate whether plugin is in Simulation Mode.
	 */
	const char* Key_Simulation_Mode = "Simulation Mode ";

    bool _simulation_mode;
    
};

}