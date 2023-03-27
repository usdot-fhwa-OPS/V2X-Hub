#include <simulation/SimulationEnvVar.h>

namespace tmx::utils::sim{
    bool is_simulation_mode() {
            std::string sim_mode = std::getenv(SIMULATION_MODE);
            if ( sim_mode.compare("true") == 0 || sim_mode.compare("TRUE") == 0) {
                return true;
            }
            return false;
    }
}