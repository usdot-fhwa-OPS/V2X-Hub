#pragma once

#include "PluginClient.h"
#include "carma-clock/carma_clock_example.h"

namespace tmx ::utils {

/**
 * A base plugin class which is can handle simulated time as needed.
*/
class PluginClientClockAware : public PluginClient {
public:
    PluginClientClockAware(const std::string & name);

protected:
    inline std::shared_ptr<fwha_stol::lib::time::CarmaClock> const getClock() {
        return clock;
    } 

private:
    std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock;
};

}