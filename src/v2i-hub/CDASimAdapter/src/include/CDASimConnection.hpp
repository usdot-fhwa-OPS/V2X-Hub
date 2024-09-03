#pragma once
#include <UdpServer.h>
#include <UdpClient.h>
#include <tmx/tmx.h>
#include <Point.h>
#include <TimeSyncMessage.h>
#include <SensorDetectedObject.h>
#include <jsoncpp/json/json.h>
#include <PluginLog.h>
#include <gtest/gtest.h>
#include <fstream>


namespace CDASimAdapter {

    
    /**
     * @brief Object to hold UdpServers and UdpClients to facilitate V2X-Hub connection
     * with CDASim (CARMA-Simulation). This includes setting up servers and clients to foward
     * V2X messages, listen for time sync messages from CDASim, and attempt Infrastructure 
     * registration with CDA Sim.
     * 
     */
    class CDASimConnection {
        public:
            /**
             * @brief Constructor. 
             * @param simulation_ip IP address of CARMA Simulation.
             * @param infrastructure_id Id of this infrastructure
             * @param simulation_registration_port Port on CARMA Simulation which handles infrastructure registration.
             * @param local_ip IP address of infrastructure software (V2X-Hub).
             * @param time_sync_port Port on which connection listens for time synchronization messages.
             * @param v2x_port Port on which connecction listens for incoming v2x messages.
             * @param location Simulationed location of infrastructure.
             */
            explicit CDASimConnection( const std::string &simulation_ip, const std::string &infrastructure_id, const uint simulation_registration_port, 
                                const uint sim_v2x_port, const std::string &local_ip,  const uint time_sync_port, const uint simulated_interaction_port,  const uint v2x_port, 
                                const tmx::utils::Point &location, const std::string &sensor_json_file_path = "");
             /**
             * @brief Method to forward v2x message to CARMA Simulation
             * @param v2x_message string
             */
            void forward_v2x_message_to_simulation(const std::string &v2x_message) const;

            /**
             * @brief Method to forward v2x message to V2X-Hub message receiver plugin.
             * @param v2x_message string
             */
            void forward_v2x_message_to_v2xhub(const std::string &v2x_message) const;
            /**
             * @brief Method to consume incoming v2x message from simulation
             * @return string of v2x messaage.
             */
            std::string consume_v2x_message_from_simulation() const;
            /**
             * @brief Method to forward message to external UDP listener via UDP Client
             * @param v2x_message string message to forward.
             * @param _client UDP client to forward message with. TMX message bus for other V2X-Hub Plug immediate forward plugin.
             * @return string of v2x messaage.
             */
            std::string consume_v2x_message_from_v2xhub() const;
            /**
             * @param v2x_message string message to forward.
             * @param _client UDP client to forward message with.
             */
            void forward_message(const std::string &v2x_message, const std::shared_ptr<tmx::utils::UdpClient> _client ) const ;

            /**
             * @brief Method to consume incoming std::string message in hex format from UDP Server.
             * @param _server UDP Server to consume string message from.
             * @return string of message.
             */
            std::string consume_hex_server_message( const std::shared_ptr<tmx::utils::UdpServer> _server) const;
            
            /**
             * @brief Method to consume incoming time sychronization message.
             * @return TimeSyncMessage.
             */
            tmx::messages::TimeSyncMessage consume_time_sync_message() const;
            /**
             * @brief Method to consume incoming external object message.
             * To populate the simulation external object, this JSON string has to follow this specification: https://usdot-carma.atlassian.net/wiki/spaces/CRMSIM/pages/2563899417/Detected+Objects+Specification#CARMA-Street-and-V2xHub
             * @return simulation::SensorDetectedObject.
             */
            tmx::messages::SensorDetectedObject consume_sensor_detected_object_message() const;
            /**
             * @brief Perform handshake with CARMA-Simulation. Will return true on successful handshakes and false if 
             * unsuccessful. As part of the handshake should set simulation_v2x_port for forwarding v2x messages to simulation,
             * initialize UDP Servers and Clients for message forwarding, and set connected property. 
             * @param simulation_ip address of CARMA-Simulation instance.
             * @param simulation_registration_port infrastructure registration port of CARMA-Simulation instance.
             * @param local_ip address of infrastructure software (V2X-Hub).
             * @param time_sync_port port assigned to listening for time sychronization messages from CARMA-Simulation.
             * @param v2x_port port assigned to listening for v2x messages from CARMA-Simulation.
             * @param location simulated location of infrastructure hardware.
             * @return true if handshake successful and false if handshake unsuccessful.
             */
            bool carma_simulation_handshake(const std::string &simulation_ip, const std::string &infrastructure_id, const uint simulation_registration_port,
                                const std::string &local_ip,  const uint time_sync_port, const uint simulated_interaction_port,  const uint v2x_port, 
                                const tmx::utils::Point &location);
            
            /**
             * @brief Method to setup UDP Servers and Clients after handshake to facilate message forwarding.
             * @param simulation_ip address of CARMA-Simulation instance.
             * @param local_ip address of infrastructure software (V2X-Hub).
             * @param time_sync_port port assigned to listen for time sychronization messages from CARMA-Simulation.
             * @param v2x_port port assigned for listening for v2x messages from CARMA-Simulation.
             * @param simulation_v2x_port port on which CARMA-Simulation is listening for incoming v2x messages.
             * @return true if setup is successful and false otherwise.
             */
            bool setup_udp_connection(const std::string &simulation_ip, const std::string &local_ip,  const uint time_sync_port, const uint simulated_interaction_port, 
                                const uint v2x_port, const uint simulation_v2x_port);
            /**
             * @brief Method to attempt to establish connection between CARMA-Simulation and infrastucture. Returns true if succesful
             * and false if unsuccessful.
             * @return returns true if succesful and false if unsuccessful.
             */
            bool connect();
            /**
             * @brief Returns true if CARMA-Simulation connection is active and false if the connection is inactive.
             * @return Returns true if CARMA-Simulation connection is active and false if the connection is inactive.
             */
            bool is_connected() const;

        private:
            /**
             * @brief helper method to generate JSON used for handshake connection with CARMA-Simulation instance. 
             * @param local_ip address of infrastructure software (V2X-Hub).
             * @param time_sync_port port assigned to listening for time sychronization messages from CARMA-Simulation.
             * @param v2x_port port assigned to listening for v2x messages from CARMA-Simulation.
             * @param location simulated location of infrastructure hardware.
             * @return true if handshake successful and false if handshake unsuccessful.
             */
            std::string get_handshake_json(const std::string &infrastructure_id, const std::string &local_ip,  const uint time_sync_port, const uint simulated_interaction_port, const uint v2x_port, 
                                const tmx::utils::Point &location) const; 
            
            /**
             * @brief Helper method to read JSON file and return resulting Json::Value. 
             * @param file_path A string of file path in the host machine to the JSON file.
             * @return A Json::Value object.
             * @throw std::invalid_argument exception if no file is found or file is not readable for provided path.
            */
            Json::Value read_json_file(const std::string& file_path) const;
            /**
             * @brief Helper method to convert string JSON content to JSON::Value.
             * @param json_str A JSON string content of Sensor Configuration JSON file
             * @return A Json::Value object parsed from string.
             * @throw std::invalid_argument exception if provided string is not valid JSON
             */
            Json::Value string_to_json(const std::string &json_str) const;
            /**
             * @brief Method to read Sensor Configuration JSON file and return Sensor Registration information
             * required for Infrastructure Registration to CDASim.
             * @param file_path Path to Sensor Configuration file.
             * @return Json::Value correspoding to Sensor Registration information for Infrastructure Registration to CDASim.
             */
            Json::Value read_sensor_configuration_file(const std::string &file_path) const;

            std::string _simulation_ip;
            uint _simulation_registration_port;
            std::string _infrastructure_id;
            uint _simulation_v2x_port;
            uint _simulated_interaction_port;
            std::string _local_ip;
            uint _time_sync_port;
            uint _v2x_port;
            tmx::utils::Point _location;
            bool _connected = false;
            std::string _sensor_json_file_path;

            std::shared_ptr<tmx::utils::UdpServer> carma_simulation_listener;
            std::shared_ptr<tmx::utils::UdpClient> carma_simulation_publisher;
            std::shared_ptr<tmx::utils::UdpClient> carma_simulation_registration_publisher;
            std::shared_ptr<tmx::utils::UdpServer> immediate_forward_listener;
            std::shared_ptr<tmx::utils::UdpClient> message_receiver_publisher;
            std::shared_ptr<tmx::utils::UdpServer> time_sync_listener;
            std::shared_ptr<tmx::utils::UdpServer> sensor_detected_object_listener;

            FRIEND_TEST(TestCDASimConnection, get_handshake_json);
            FRIEND_TEST(TestCDASimConnection, get_handshake_json_no_sensor_config);
            FRIEND_TEST(TestCDASimConnection, read_json_file);
            FRIEND_TEST(TestCDASimConnection, string_to_json);

            
    };

}
