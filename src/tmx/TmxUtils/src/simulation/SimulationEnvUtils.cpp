#include "simulation/SimulationEnvUtils.h"

namespace tmx::utils::sim{
    bool is_simulation_mode() {
        auto sim_mode = std::getenv(SIMULATION_MODE);
        if ( sim_mode) {
            std::string sim_mode_string = sim_mode;
            if ( sim_mode_string.compare("true") == 0 || sim_mode_string.compare("TRUE") == 0) {
                return true;
            }
        }
        return false;

    }

    std::string get_sim_config(const char *config_name, bool required) {
        if (is_simulation_mode() && config_name) {
            if ( std::getenv(config_name) != nullptr) {
                std::string config =  std::getenv(config_name);
                return config;
            }
            else {
                std::string config_name_str = config_name;
                if ( required ) {
                    BOOST_THROW_EXCEPTION(TmxException("Required simulation config " + config_name_str + " not set!"));
                }
            }
        } else {
            BOOST_THROW_EXCEPTION(TmxException("V2X-Hub not in sumulation mode or config param name is null pointer!"));
        }
        return "";
    }
}