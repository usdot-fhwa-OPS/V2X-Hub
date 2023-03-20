#include "include/CARMASimulationAdapter.hpp"


using namespace tmx::utils;

namespace CARMASimulationAdapter{

    CARMASimulationAdapter::CARMASimulationAdapter(const std::string &name) : PluginClientClockAware(name){
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
            // While CARMA Simulation connection is down, attempt to reconnect
            while ( !connection || !connection->is_connected() ) {
                connect();
            }
        }
    }

    bool CARMASimulationAdapter::initialize_time_producer() {
        std::string _broker_str = std::getenv(KAFKA_BROKER_ADDRESS_ENV);
        std::string _topic = std::getenv(TIME_SYNC_TOPIC_ENV);

        kafka_client client;
        time_producer =  client.create_producer(_broker_str,_topic);
    }

    bool CARMASimulationAdapter::connect() {
        std::string _simulation_ip = std::getenv(SIMULATION_IP_ENV);
        std::string _local_ip = std::getenv(LOCAL_IP_ENV);

        uint _simulation_registration_port = std::stoul(std::getenv(SIMULATION_REGISTRATION_PORT_ENV));
        uint _time_sync_port = std::stoul(std::getenv(TIME_SYNC_PORT_ENV));
        uint _v2x_port = std::stoul(std::getenv(V2X_PORT_ENV));
        if (!initialize_time_producer()) {
            return false;
        }
        if ( connection )
            connection.reset(new CARMASimulationConnection( simulation_ip, simulation_registration_port,_local_ip,
                                                         _time_sync_port, _v2x_port, location, time_producer ));
        else {
            connection = std::make_unique<CARMASimulationConnection>(simulation_ip, simulation_registration_port,_local_ip,
                                                         _time_sync_port, _v2x_port, location, time_producer);
        }
        return connection->connect();
    }
    
    int CARMASimulationAdapter::Main() {


        PLOG(logINFO) << "Starting plugin.";		

        while (_plugin->state != IvpPluginState_error) {

            if (IsPluginState(IvpPluginState_registered))
            {
                
            }
        }

	    return EXIT_SUCCESS;
    }


    
        

}
int main(int argc, char *argv[]) {
	return run_plugin < CARMASimulationAdapter::CARMASimulationAdapter > ("CARMASimulationAdapter", argc, argv);
}