#include "include/CDASimAdapter.hpp"


using namespace tmx::utils;

namespace CDASimAdapter{

    CDASimAdapter::CDASimAdapter(const std::string &name) : PluginClient(name){
        if (!sim::is_simulation_mode()) {
            throw TmxException("CDASimAdapter only necessary in simulation mode!");
        }

    }

    void CDASimAdapter::UpdateConfigSettings() {
        GetConfigValue<double>("Longitude", location.Longitude);
        GetConfigValue<double>("Latitude", location.Latitude);
        GetConfigValue<double>("Elevation", location.Elevation);
        PLOG(logINFO) << "Location of Simulated V2X-Hub updated to : {" << location.Longitude << ", " 
            << location.Latitude << ", " << location.Elevation << "}." << std::endl;
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
        try {
            std::string _broker_str = sim::get_sim_config(sim::KAFKA_BROKER_ADDRESS);
            std::string _topic = sim::get_sim_config(sim::TIME_SYNC_TOPIC);

            kafka_client client;
            time_producer =  client.create_producer(_broker_str,_topic);
            return time_producer->init();

        }
        catch( const runtime_error &e ) {
            PLOG(logWARNING) << "Initialization of time producer failed: " << e.what() << std::endl; 
        }
        return false;

    }

    void CDASimAdapter::forward_time_sync_message(tmx::messages::TimeSyncMessage &msg) {
        std::string payload =msg.to_string();
        PLOG(logDEBUG1) << "Sending Time Sync Message " << msg << std::endl;
        this->BroadcastMessage<tmx::messages::TimeSyncMessage>(msg, _name, 0 , IvpMsgFlags_None);
        if (time_producer && time_producer->is_running()) {
            try {
                time_producer->send(payload);
            }
            catch( const runtime_error &e ) {
                PLOG(logERROR) << "Exception encountered during kafka time sync forward : " << e.what() << std::endl;
            }
        }
        
    }

    bool CDASimAdapter::connect() {
        try {
            std::string _simulation_ip = sim::get_sim_config(sim::SIMULATION_IP);
            std::string _local_ip =  sim::get_sim_config(sim::LOCAL_IP);
            PLOG(logERROR) << "Simulation and local IP successfully initialized!"<< std::endl;
            uint _simulation_registration_port = std::stoul(sim::get_sim_config(sim::SIMULATION_REGISTRATION_PORT));
            uint _time_sync_port = std::stoul(sim::get_sim_config(sim::TIME_SYNC_PORT));
            uint _v2x_port = std::stoul(sim::get_sim_config(sim::V2X_PORT));
            uint _sim_v2x_port = std::stoul(sim::get_sim_config(sim::SIM_V2X_PORT));
            uint _infrastructure_id;
            // = std::stoul(sim::get_sim_config(sim::INFRASTRUCTURE_ID));

            PLOG(logINFO) << "CDASim connecting " << _simulation_ip << 
                    "\nUsing Registration Port : "  << std::to_string( _simulation_registration_port) <<
                    " Time Sync Port: " << std::to_string( _time_sync_port) << " and V2X Port: " << std::to_string(_v2x_port) << std::endl;
            if (!initialize_time_producer()) {
                return false;
            }
            if ( connection ) {
                connection.reset(new CDASimConnection( _simulation_ip, _infrastructure_id, _simulation_registration_port, _sim_v2x_port, _local_ip,
                                                _time_sync_port, _v2x_port, location ));
            }
            else {
                connection = std::make_unique<CDASimConnection>(_simulation_ip, _infrastructure_id, _simulation_registration_port, _sim_v2x_port, _local_ip,
                                                            _time_sync_port, _v2x_port, location);
            }
        }       
        catch (const TmxException &e) {
            PLOG(logERROR) << "Exception occured attempting to initialize CDASim Connection : " << e.what() << std::endl;
            return false;
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
        PLOG(logDEBUG) << "Creating Thread Timer for time sync" << std::endl;
        if ( !thread_timer ) {
            thread_timer = std::make_unique<tmx::utils::ThreadTimer>();
        }
        time_sync_tick_id = thread_timer->AddPeriodicTick([this]() {
            PLOG(logDEBUG1) << "Listening for time sync messages from CDASim." << std::endl;
            this->attempt_time_sync();
        } // end of lambda expression
        , std::chrono::milliseconds(100));
        thread_timer->Start();
    }

    void CDASimAdapter::attempt_time_sync() {
        try {
            tmx::messages::TimeSyncMessage msg = connection->consume_time_sync_message();
            if ( !msg.is_empty()) {
                forward_time_sync_message( msg );
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