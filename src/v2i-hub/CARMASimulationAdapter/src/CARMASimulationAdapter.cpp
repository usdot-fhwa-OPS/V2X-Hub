#include <CARMASimulationAdapter.hpp>


using namespace tmx::utils;

namespace CARMASimulationAdapter{

    CARMASimulationAdapter::CARMASimulationAdapter(std::string name) : PluginClientClockAware(name){
        PLOG(logDEBUG1) << "Initialize " << name << " plugin!" << std::endl;
    }

    CARMASimulationAdapter::~CARMASimulationAdapter() {
        PLOG(logDEBUG1) << "Destroy " << _name << " plugin!" << std::endl;
    }

    void CARMASimulationAdapter::UpdateConfigSettings() {
        // TODO
    }

    void CARMASimulationAdapter::OnConfigChanged(const char *key, const char *value) {
        PluginClient::OnConfigChanged(key, value);
	    UpdateConfigSettings();    
    }

    void CARMASimulationAdapter::OnStateChange(IvpPluginState state) {
        PluginClient::OnStateChange(state);

        if (state == IvpPluginState_registered) {
            UpdateConfigSettings();
        }
    }

    bool CARMASimulationAdapter::initialize_time_producer() {
        std::string _broker_str = std::getenv(KAFKA_BROKER_ADDRESS_ENV);
    }
    
        

}