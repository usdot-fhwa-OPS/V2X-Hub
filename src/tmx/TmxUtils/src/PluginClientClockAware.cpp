#include "PluginClientClockAware.h"

#include "jsoncpp/json/json.h"

namespace tmx::utils {

    PluginClientClockAware::PluginClientClockAware(const std::string & name)
        : PluginClient(name)
    {
        // check for simulation mode enabled by environment variable
        bool simulationMode = sim::is_simulation_mode();

        using namespace fwha_stol::lib::time;
        clock = std::make_shared<CarmaClock>(simulationMode);
        if (simulationMode) {
            AddMessageFilter<tmx::messages::TimeSyncMessage>(this, &PluginClientClockAware::HandleTimeSyncMessage);

        }

    }


    void PluginClientClockAware::HandleTimeSyncMessage(tmx::messages::TimeSyncMessage &msg, routeable_message &routeableMsg ) {
       clock->update( msg.get_timestamp() );
    }

}