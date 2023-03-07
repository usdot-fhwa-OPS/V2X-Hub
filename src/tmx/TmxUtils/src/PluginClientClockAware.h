#pragma once

#include "PluginClient.h"
#include "carma-clock/carma_clock.h"

namespace tmx ::utils {

/**
 * A base plugin class which is can handle simulated time as needed.
*/
class PluginClientClockAware : public PluginClient {
public:
    explicit PluginClientClockAware(const std::string & name);

protected:
    inline std::shared_ptr<fwha_stol::lib::time::CarmaClock> getClock() const {
        return clock;
    } 

private:
    std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock;
};

}