#include "PluginClientClockAware.h"

namespace tmx ::utils {

PluginClientClockAware::PluginClientClockAware(const std::string & name)
    : PluginClient(name)
{
    // run a test for now
    using namespace fwha_stol::lib::time;
    clock = std::make_shared<CarmaClock>(true);
    std::thread t([this]() {
        // test waiting for initialization
        this_thread::sleep_for(chrono::seconds(10));
        for (auto i = 0; i < 1000; i++) {
            // move 100 milliseconds every half second
            clock->update(i*100);
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    });
    t.detach();
}

}