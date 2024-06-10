#include "include/CDASimAdapter.hpp"
#include <chrono>

using namespace tmx::utils;

namespace CDASimAdapter{

    CDASimAdapter::CDASimAdapter(const std::string &name) : PluginClient(name){
        if (!sim::is_simulation_mode()) {
            throw TmxException("CDASimAdapter only necessary in simulation mode!");
        }

    }

    void CDASimAdapter::UpdateConfigSettings() {
        std::scoped_lock<std::mutex> lock(_lock);
        bool success = false;
        success = GetConfigValue<double>("X", location.X);
        success = success && GetConfigValue<double>("Y", location.Y);
        success = success && GetConfigValue<double>("Z", location.Z);
        PLOG(logINFO) << "Location of Simulated V2X-Hub updated to : {" << location.X << ", "
            << location.Y << ", " << location.Z << "}." << std::endl;
        success = success && GetConfigValue<int>("MaxConnectionAttempts", max_connection_attempts);
        success = success && GetConfigValue<uint>("ConnectionSleepTime", connection_sleep_time);
        if (connection_sleep_time < 1 ) {
            PLOG(logWARNING) << "ConnectionSleepTime of " << connection_sleep_time << " is invalid. Valid values are <= 1." << std::endl;
            connection_sleep_time = 1;
        }
        if (!success) {
            PLOG(logWARNING) << "Some configuration parameters were not successfully loaded! Please ensure configuration parameter keys are correct!" << std::endl;
        }
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
            int connection_attempts = 0;
            while ( (!connection || !connection->is_connected()) && (connection_attempts < max_connection_attempts || max_connection_attempts < 1 ) ) {
                PLOG(logINFO) << "Attempting CDASim connection " << connection_attempts << "/" << max_connection_attempts << " ..." << std::endl;
                bool success = connect();
                if (success) {
                    PLOG(logINFO) << "Connection to CDASim established!" << std::endl;
                }
                connection_attempts++;
                // Sleep for configurable seconds in between connection attempts. No sleep is required on final failing attempt
                if ( !connection->is_connected() && (connection_attempts < max_connection_attempts || max_connection_attempts < 1 ) ) {
                    PLOG(logDEBUG) << "Sleeping for " << connection_sleep_time << " seconds before next connection attempt ..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(connection_sleep_time));
                }
            }

            if ( connection->is_connected() ) {
                start_time_sync_thread_timer();
                start_sensor_detected_object_detection_thread();
                start_immediate_forward_thread();
                start_message_receiver_thread();
            }else {
                PLOG(logERROR) << "CDASim connection failed!" << std::endl;
            }

        }
    }



    void CDASimAdapter::forward_time_sync_message(tmx::messages::TimeSyncMessage &msg) {

        std::string payload =msg.to_string();
        // A script to validate time synchronization of tools in CDASim currently relies on the following
        // log line. TODO: This line is meant to be removed in the future upon completion of this work:
        // https://github.com/usdot-fhwa-stol/carma-analytics-fotda/pull/43
        auto time_now = std::chrono::system_clock::now();
        auto epoch = time_now.time_since_epoch();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        PLOG(logDEBUG1) << "Simulation Time: " << msg.get_timestep() << " where current system time is: " << milliseconds.count() << ", where msgs: " << msg << std::endl;

        this->BroadcastMessage<tmx::messages::TimeSyncMessage>(msg, _name, 0 , IvpMsgFlags_None);

    }

    void CDASimAdapter::forward_simulated_detected_message(tmx::messages::simulation::SensorDetectedObject &msg) {
        PLOG(logDEBUG1) << "Sending Simulated SensorDetectedObject Message " << msg << std::endl;
        this->BroadcastMessage<tmx::messages::simulation::SensorDetectedObject>(msg, _name, 0 , IvpMsgFlags_None);
    }

    bool CDASimAdapter::connect() {
        try {
            std::string simulation_ip = sim::get_sim_config(sim::SIMULATION_IP);
            std::string local_ip =  sim::get_sim_config(sim::LOCAL_IP);
            PLOG(logINFO) << "Simulation and local IP successfully initialized!"<< std::endl;
            uint simulation_registration_port = std::stoul(sim::get_sim_config(sim::SIMULATION_REGISTRATION_PORT));
            uint time_sync_port = std::stoul(sim::get_sim_config(sim::TIME_SYNC_PORT));
            auto simulated_interaction_port =static_cast<unsigned int>(std::stoi(sim::get_sim_config(sim::SIM_INTERACTION_PORT)));
            uint v2x_port = std::stoul(sim::get_sim_config(sim::V2X_PORT));
            uint sim_v2x_port = std::stoul(sim::get_sim_config(sim::SIM_V2X_PORT));
            std::string infrastructure_id = sim::get_sim_config(sim::INFRASTRUCTURE_ID);
            // Sensor JSON file path is an optional environment variable that allows configuration of
            // simulated sensor if provided.
            std::string sensor_json_file_path = "";
            sensor_json_file_path = sim::get_sim_config(sim::SENSOR_JSON_FILE_PATH, false);

            PLOG(logINFO) << "CDASim connecting " << simulation_ip <<
                    "\nUsing Registration Port : "  << std::to_string( simulation_registration_port) <<
                    " Time Sync Port: " << std::to_string( time_sync_port) << " and V2X Port: " << std::to_string(v2x_port) << std::endl;
            if ( connection ) {
                if ( sensor_json_file_path.empty()) {
                    connection.reset(new CDASimConnection( simulation_ip, infrastructure_id, simulation_registration_port, sim_v2x_port, local_ip,
                                                time_sync_port, simulated_interaction_port, v2x_port, location ));
                }
                else {
                    connection.reset(new CDASimConnection( simulation_ip, infrastructure_id, simulation_registration_port, sim_v2x_port, local_ip,
                                                time_sync_port, simulated_interaction_port, v2x_port, location, sensor_json_file_path ));
                }
            }
            else {
                if ( sensor_json_file_path.empty()) {
                    connection = std::make_unique<CDASimConnection>(simulation_ip, infrastructure_id, simulation_registration_port, sim_v2x_port, local_ip,
                                                            time_sync_port, simulated_interaction_port, v2x_port, location);
                }
                else {
                    connection = std::make_unique<CDASimConnection>(simulation_ip, infrastructure_id, simulation_registration_port, sim_v2x_port, local_ip,
                                                            time_sync_port, simulated_interaction_port, v2x_port, location, sensor_json_file_path);
                }
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



    void CDASimAdapter::start_immediate_forward_thread() {
        if ( !immediate_forward_timer ) {
            immediate_forward_timer = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(5));
        }
        immediate_forward_tick_id = immediate_forward_timer->AddPeriodicTick([this]() {
            this->attempt_message_from_v2xhub();

        } // end of lambda expression
        , std::chrono::milliseconds(5) );

        immediate_forward_timer->Start();

    }

    void CDASimAdapter::start_message_receiver_thread() {
        if ( !message_receiver_timer ) {
            message_receiver_timer = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(5));
        }

        message_receiver_tick_id = message_receiver_timer->AddPeriodicTick([this]() {
            this->attempt_message_from_simulation();
        } // end of lambda expression
        , std::chrono::milliseconds(5) );
        message_receiver_timer->Start();

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

    void CDASimAdapter::start_sensor_detected_object_detection_thread() {
        PLOG(logDEBUG) << "Creating Thread Timer for simulated external object" << std::endl;
        try
        {
            if(!external_object_detection_thread_timer)
            {
                external_object_detection_thread_timer  = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(5));
            }
            external_object_detection_thread_timer->AddPeriodicTick([this](){
                PLOG(logDEBUG1) << "Listening for Sensor Detected Message from CDASim." << std::endl;
                auto msg = connection->consume_sensor_detected_object_message();
                if ( !msg.is_empty()) {
                    this->forward_simulated_detected_message(msg);
                }
                else
                {
                    PLOG(logDEBUG1) << "CDASim connection has not yet received an simulated sensor detected message!" << std::endl;
                }
            }//End lambda
            , std::chrono::milliseconds(5));
            external_object_detection_thread_timer->Start();
        }
        catch ( const UdpServerRuntimeError &e )
        {
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
        if ( !time_sync_timer ) {
            time_sync_timer = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(5));
        }
        time_sync_tick_id = time_sync_timer->AddPeriodicTick([this]() {
            PLOG(logDEBUG1) << "Listening for time sync messages from CDASim." << std::endl;
            this->attempt_time_sync();
        } // end of lambda expression
        , std::chrono::milliseconds(5));
        time_sync_timer->Start();
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