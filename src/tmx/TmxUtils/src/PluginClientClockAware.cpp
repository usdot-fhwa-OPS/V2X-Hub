#include "PluginClientClockAware.h"

#include "jsoncpp/json/json.h"

namespace tmx::utils {

    PluginClientClockAware::PluginClientClockAware(const std::string & name)
        : PluginClient(name)
    {
        // check for simulation mode enabled by environment variable
        _simulation_mode = sim::is_simulation_mode();

        using namespace fwha_stol::lib::time;
        clock = std::make_shared<CarmaClock>(_simulation_mode);
        if (_simulation_mode) {
            AddMessageFilter<tmx::messages::TimeSyncMessage>(this, &PluginClientClockAware::HandleTimeSyncMessage);
            PLOG(logDEBUG2) << "Added Time Sync Message Filter";
        }

    }


    void PluginClientClockAware::HandleTimeSyncMessage(tmx::messages::TimeSyncMessage &msg, routeable_message &routeableMsg ) {
        if (_simulation_mode ) {
            PLOG(logDEBUG3) << "Message Received " << msg.to_string() << std::endl;
            clock->update( msg.get_timestep() );
            SetStatus(Key_Simulation_Time_Step, Clock::ToUtcPreciseTimeString(msg.get_timestep()));
        }
    }

    void PluginClientClockAware::OnStateChange(IvpPluginState state) {
        PluginClient::OnStateChange(state);
        if (state == IvpPluginState_registered && sim::is_simulation_mode()) {
            SetStatus(Key_Simulation_Mode, "ON");
        }
    }

    bool PluginClientClockAware::isSimulationMode() const {
        return _simulation_mode;
    }

    std::shared_ptr<fwha_stol::lib::time::CarmaClock> PluginClientClockAware::getClock() const {
        clock->wait_for_initialization(); // Blocks until first call to update when in sim mode.
        return clock;
    }

}
