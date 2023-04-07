#include "include/CDASimAdapter.hpp"


using namespace tmx::utils;

namespace CDASimAdapter{

    int CDASimAdapter::infrastructure_id = 0;

    CDASimAdapter::CDASimAdapter(const std::string &name) : PluginClientClockAware(name), infrastructure_id_adapter (get_next_id()){
        PLOG(logDEBUG1) << "Initialize " << name << " plugin! with ID: "  << infrastructure_id_adapter << std::endl;
    }

    void CDASimAdapter::UpdateConfigSettings() {
        // TODO
    }

    void CDASimAdapter::OnConfigChanged(const char *key, const char *value) {
        PluginClient::OnConfigChanged(key, value);
	    UpdateConfigSettings();    
    }

    void CDASimAdapter::OnStateChange(IvpPluginState state) {
        PluginClient::OnStateChange(state);

        if (state == IvpPluginState_registered) {
            UpdateConfigSettings();
            // While CARMA Simulation connection is down, attempt to reconnect
            while ( !connection || !connection->is_connected() ) {
                connect();
            }
        }
    }

    bool CDASimAdapter::initialize_time_producer() {
        std::string _broker_str = std::getenv(sim::KAFKA_BROKER_ADDRESS);
        std::string _topic = std::getenv(sim::TIME_SYNC_TOPIC);

        try {
            kafka_client client;
            time_producer =  client.create_producer(_broker_str,_topic);
        }
        catch( const runtime_error &e ) {
            PLOG(logWARNING) << "Initialization of time producer failed: " << e.what() << std::endl; 
            return false;
        }
        return true;

    }

    bool CDASimAdapter::connect() {
        std::string _simulation_ip = std::getenv(sim::SIMULATION_IP);
        std::string _local_ip = std::getenv(sim::LOCAL_IP);

        uint _simulation_registration_port = std::stoul(std::getenv(sim::SIMULATION_REGISTRATION_PORT));
        uint _time_sync_port = std::stoul(std::getenv(sim::TIME_SYNC_PORT));
        uint _v2x_port = std::stoul(std::getenv(sim::SIM_V2X_PORT));
        if (!initialize_time_producer()) {
            return false;
        }
        if ( connection )
            connection.reset(new CDASimConnection( simulation_ip, infrastructure_id_adapter, simulation_registration_port,_local_ip,
                                                         _time_sync_port, _v2x_port, location, time_producer ));
        else {
            connection = std::make_unique<CDASimConnection>(simulation_ip, infrastructure_id_adapter, simulation_registration_port,_local_ip,
                                                         _time_sync_port, _v2x_port, location, time_producer);
        }
        return connection->connect();
    }
    
    int CDASimAdapter::Main() {


        PLOG(logINFO) << "Starting plugin " << _name << std::endl;		

        while (_plugin->state != IvpPluginState_error) {

            if (IsPluginState(IvpPluginState_registered))
            {
                
            }
        }

	    return EXIT_SUCCESS;
    }


    
        

}
int main(int argc, char *argv[]) {
	return run_plugin < CDASimAdapter::CDASimAdapter > ("CDASimAdapter", argc, argv);
}