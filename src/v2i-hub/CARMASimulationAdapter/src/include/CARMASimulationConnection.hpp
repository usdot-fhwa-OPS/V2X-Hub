#pragma once
#include <UdpServer.h>
#include <UdpClient.h>
#include <tmx/tmx.h>
#include <WGS84Point.h>
#include <kafka_producer_worker.hpp>
#include <byte_stream.hpp>


namespace CARMASimulationAdapter {

    struct Time_Sync_Message
    {
        uint64_t cur_time;
    };
    

    class CARMASimulationConnection {
        public:
            /**
             * @brief Constructor. 
             * @param simulation_ip IP address of CARMA Simulation.
             * @param simulation_registration_port Port on CARMA Simulation which handles infrastructure registration.
             * @param local_ip IP address of infrastructure software (V2X-Hub).
             * @param time_sync_port Port on which connection listens for time synchronization messages.
             * @param v2x_port Port on which connecction listens for incoming v2x messages.
             * @param location Simulationed location of infrastructure.
             * @param producer Kafka Producer for forwarding time synchronization messages.
             */
            CARMASimulationConnection( const std::string &simulation_ip, const uint simulation_registration_port, 
                                const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                const tmx::utils::WGS84Point &location, std::shared_ptr<kafka_clients::kafka_producer_worker> time_producer);
            ~CARMASimulationConnection();

             /**
             * @brief Method to forward v2x message to CARMA Simulation
             * @param v2x_message byte_stream
             */
            void forward_v2x_message_to_simulation(const tmx::byte_stream &v2x_message) const;

            /**
             * @brief Method to forward v2x message to V2X-Hub message receiver plugin.
             * @param v2x_message byte_stream
             */
            void foward_v2x_message_to_v2xhub(const tmx::byte_stream &v2x_message) const;

            /**
             * @brief Method to consume incoming v2x message from simulation
             * @return byte_stream of v2x messaage.
             */
            tmx::byte_stream consume_v2x_message_from_simulation() const;
            
            /**
             * @brief Method to consume incoming v2x message from V2X-Hub immediate forward plugin.
             * @return byte_stream of v2x messaage.
             */
            tmx::byte_stream consume_v2x_message_from_v2xhub() const;
            /**
             * @brief Forward time sychronization message to infrastructure Kafka Broker
             * @param msg Time synchronization message.
             */
            void forward_time_sync_message(const Time_Sync_Message &msg ) const;
            
            /**
             * @brief Method to consume incoming time sychronization message.
             * @return Time_Sync_Message.
             */
            Time_Sync_Message consume_time_sync_message() const;
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
            bool carma_simulation_handshake(const std::string &simulation_ip, const uint simulation_registration_port, 
                                const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                const tmx::utils::WGS84Point &location);
            
            /**
             * @brief Method to setup UDP Servers and Clients after handshake to facilate message forwarding.
             * @param simulation_ip address of CARMA-Simulation instance.
             * @param local_ip address of infrastructure software (V2X-Hub).
             * @param time_sync_port port assigned to listen for time sychronization messages from CARMA-Simulation.
             * @param v2x_port port assigned for listening for v2x messages from CARMA-Simulation.
             * @param simulation_v2x_port port on which CARMA-Simulation is listening for incoming v2x messages.
             * @return true 
             */
            bool setup_udp_connection(const std::string &simulation_ip, const std::string &local_ip,  const uint time_sync_port, 
                                const uint v2x_port, const uint simulation_v2x_port);
        private:
            std::string _simulation_ip;
            uint _simulation_registration_port;
            std::string _local_ip;
            uint _time_sync_port;
            uint _v2x_port;
            tmx::utils::WGS84Point _location;
            uint _simulation_v2x_port;
            bool _connected;
            std::unique_ptr<tmx::utils::UdpServer> carma_simulation_listener;
            std::unique_ptr<tmx::utils::UdpClient> carma_simulation_publisher;
            std::unique_ptr<tmx::utils::UdpServer> immediate_forward_listener;
            std::unique_ptr<tmx::utils::UdpClient> message_receiver_publisher;
            std::shared_ptr<RdKafka::Producer> time_sync_producer;
    }

}