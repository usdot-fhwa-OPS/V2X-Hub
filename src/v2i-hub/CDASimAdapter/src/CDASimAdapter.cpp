#include "include/CDASimAdapter.hpp"


using namespace tmx::utils;

namespace CDASimAdapter{

    CDASimAdapter::CDASimAdapter(const std::string &name) : PluginClientClockAware(name){
        PLOG(logDEBUG1) << "Initialize " << name << " plugin!" << std::endl;
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
            if ( connection->is_connected() ) {
                start_time_sync_thread_timer();
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

    void CDASimAdapter::forward_time_sync_message(tmx::messages::TimeSyncMessage &msg) {
        this->BroadcastMessage<tmx::messages::TimeSyncMessage>(msg, _name, 0 , IvpMsgFlags_None);
        if (time_producer && time_producer->is_running()) {
            time_producer->send(msg.to_string());
        }
        
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
        if ( connection ) {
            connection.reset(new CDASimConnection( simulation_ip, simulation_registration_port,_local_ip,
                                                _time_sync_port, _v2x_port, location ));
        }
        else {
            connection = std::make_unique<CDASimConnection>(simulation_ip, simulation_registration_port,_local_ip,
                                                         _time_sync_port, _v2x_port, location);
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


    void CDASimAdapter::start_time_sync_thread_timer() {
        if ( !thread_timer ) {
            thread_timer = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(100));
        }
        time_sync_tick_id = thread_timer->AddPeriodicTick([this]() {
            
            this->attempt_time_sync();
        } // end of lambda expression
        , std::chrono::milliseconds(100), std::chrono::milliseconds(100) );
    }

    void CDASimAdapter::attempt_time_sync() {
        try {
            tmx::messages::TimeSyncMessage msg = connection->consume_time_sync_message();
            if ( !msg.is_empty()) {
                forward_time_sync_message( msg );
            }
            else {
                PLOG(logDEBUG1) << "CDASim connection has not yet received a time sync message!" << std::endl;
            }
        }
        catch ( const UdpServerRuntimeError &e ) {
            PLOG(logERROR) << "Error occured :" << e.what() <<  std::endl;
        }
    }
        

}
int main(int argc, char *argv[]) {
	return run_plugin < CDASimAdapter::CDASimAdapter > ("CDASimAdapter", argc, argv);
}