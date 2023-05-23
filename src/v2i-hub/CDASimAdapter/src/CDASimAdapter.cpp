#include "include/CDASimAdapter.hpp"


using namespace tmx::utils;

namespace CDASimAdapter{

    CDASimAdapter::CDASimAdapter(const std::string &name) : PluginClient(name){
        if (!sim::is_simulation_mode()) {
            throw TmxException("CDASimAdapter only necessary in simulation mode!");
        }

    }

    void CDASimAdapter::UpdateConfigSettings() {
        GetConfigValue<double>("X", location.Z);
        GetConfigValue<double>("Y", location.Y);
        GetConfigValue<double>("Z", location.X);
        PLOG(logINFO) << "Location of Simulated V2X-Hub updated to : {" << location.X << ", " 
            << location.Y << ", " << location.Z << "}." << std::endl;
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
                bool success = connect();
                // Sleep for 2 seconds in between connection attempts
                if ( !connection->is_connected() ) {
                    sleep(2);
                }
            }

            if ( connection->is_connected() ) {
                start_time_sync_thread_timer();
                start_amf_msg_thread();
                start_binary_msg_thread();
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
            std::string simulation_ip = sim::get_sim_config(sim::SIMULATION_IP);
            std::string local_ip =  sim::get_sim_config(sim::LOCAL_IP);
            PLOG(logINFO) << "Simulation and local IP successfully initialized!"<< std::endl;
            uint simulation_registration_port = std::stoul(sim::get_sim_config(sim::SIMULATION_REGISTRATION_PORT));
            uint time_sync_port = std::stoul(sim::get_sim_config(sim::TIME_SYNC_PORT));
            uint v2x_port = std::stoul(sim::get_sim_config(sim::V2X_PORT));
            uint sim_v2x_port = std::stoul(sim::get_sim_config(sim::SIM_V2X_PORT));
            std::string infrastructure_id = sim::get_sim_config(sim::INFRASTRUCTURE_ID);

            PLOG(logINFO) << "CDASim connecting " << simulation_ip << 
                    "\nUsing Registration Port : "  << std::to_string( simulation_registration_port) <<
                    " Time Sync Port: " << std::to_string( time_sync_port) << " and V2X Port: " << std::to_string(v2x_port) << std::endl;
            if (!initialize_time_producer()) {
                return false;
            }
            if ( connection ) {
                connection.reset(new CDASimConnection( simulation_ip, infrastructure_id, simulation_registration_port, sim_v2x_port, local_ip,
                                                time_sync_port, v2x_port, location ));
            }
            else {
                connection = std::make_unique<CDASimConnection>(simulation_ip, infrastructure_id, simulation_registration_port, sim_v2x_port, local_ip,
                                                            time_sync_port, v2x_port, location);
            }
        }       
        catch (const TmxException &e) {
            PLOG(logERROR) << "Exception occured attempting to initialize CDASim Connection : " << e.what() << std::endl;
            return false;
        }
        catch (const std::invalid_argument &e ) {
            // std::stoul throws invalid arguement exception when provided with a string that contains characters that are not numbers.
            PLOG(logERROR) << "Exception occured attempting to initialize CDASim Connection : " << e.what() << 
                ". Check environment variables are set to the correct type!";
            return false;
        }   
        return connection->connect();
    }
    


    void CDASimAdapter::start_amf_msg_thread() {
        if ( !amf_thread_timer ) {
            amf_thread_timer = std::make_unique<tmx::utils::ThreadTimer>();
        }
        amf_msg_tick_id = amf_thread_timer->AddPeriodicTick([this]() {
            this->attempt_message_from_v2xhub();
            
        } // end of lambda expression
        , std::chrono::milliseconds(100) );
        
        amf_thread_timer->Start();

    }

    void CDASimAdapter::start_binary_msg_thread() {
        if ( !binary_thread_timer ) {
            binary_thread_timer = std::make_unique<tmx::utils::ThreadTimer>();
        }

        binary_msg_tick_id = binary_thread_timer->AddPeriodicTick([this]() {
            this->attempt_message_from_simulation();
        } // end of lambda expression
        , std::chrono::milliseconds(100) );
        binary_thread_timer->Start();

    }

    void CDASimAdapter::attempt_message_from_simulation() const {
        try {
            std::string msg = connection->consume_v2x_message_from_simulation();
            PLOG(logDEBUG1) << "binary Msg: " << msg << "of size: " << msg.size() << std::endl;
            if ( !msg.empty()) {
                connection->forward_v2x_message_to_v2xhub(msg);
                PLOG(logDEBUG1) << "Msg Forwarded to v2xhub!" << std::endl;
            }
            else {
                PLOG(logDEBUG1) << "CDASim connection has not yet received a v2x message!" << std::endl;
            }
        }
        catch ( const UdpServerRuntimeError &e ) {
            PLOG(logERROR) << "Error occured :" << e.what() <<  std::endl;
        }
    }

    void CDASimAdapter::attempt_message_from_v2xhub() const {
        try {
            std::string msg = connection->consume_v2x_message_from_v2xhub();
            PLOG(logDEBUG1) << "AMF Msg: " << msg << std::endl;
            if ( !msg.empty()) {
                connection->forward_v2x_message_to_simulation(msg);
                PLOG(logDEBUG1) << "Msg Forwarded to simulation!" << std::endl;
            }
            else {
                PLOG(logDEBUG1) << "CDASim connection has not yet received a v2x message!" << std::endl;
            }
        }
        catch ( const UdpServerRuntimeError &e ) {
            PLOG(logERROR) << "Error occured :" << e.what() <<  std::endl;
        }
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