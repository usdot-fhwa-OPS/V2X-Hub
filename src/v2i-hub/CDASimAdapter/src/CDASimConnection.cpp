#include "include/CDASimConnection.hpp"


using namespace tmx::utils;

namespace CDASimAdapter{ 
    CDASimConnection::CDASimConnection(const std::string &simulation_ip, const uint infrastructure_id, const uint simulation_registration_port, 
                                                        const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                                        const WGS84Point &location, 
                                                        std::shared_ptr<kafka_producer_worker> time_producer) : 
                                                        _simulation_ip(simulation_ip), _infrastructure_id(infrastructure_id), _simulation_registration_port(simulation_registration_port),
                                                        _local_ip(local_ip), _time_sync_port(time_sync_port), _v2x_port(v2x_port),
                                                        _location(location), _time_producer(time_producer)  {
        PLOG(logDEBUG) << "CARMA-Simulation connection initialized." << std::endl;                                                     
    } 

 

    bool CDASimConnection::is_connected() const {
        return _connected;
    }

    bool CDASimConnection::connect() {
        if (!carma_simulation_handshake(_simulation_ip, _infrastructure_id, _simulation_registration_port, _local_ip, _time_sync_port, _v2x_port, _location)) {
            _connected = false;
            return _connected;
        }
        if (!setup_udp_connection(_simulation_ip, _local_ip, _time_sync_port, _v2x_port, _simulation_v2x_port )) {
            _connected = false;
            return _connected;
        }
        if (!_time_producer || !_time_producer->is_running()) {
            _connected = false;
            return _connected;
        }
        _connected =true;
        PLOG(logINFO) << "CARMA-Simulation connection is successful!" << std::endl;
        return _connected;
    }
    bool CDASimConnection::carma_simulation_handshake(const std::string &simulation_ip, const uint infrastructure_id, const uint simulation_registration_port, 
                                const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                const WGS84Point &location) 
    {
        
        //Get remote endpoint
        try
        {
            remote_udp_ep_ = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(simulation_ip), simulation_registration_port);
        }
        catch(std::exception e)
        {
            PLOG(logERROR) << "Encountered runtime error when resolving remote udp end point given: " << e.what() << std::endl;
            throw e;
        }

        io_.reset(new boost::asio::io_service());
        udp_out_socket_.reset(new boost::asio::ip::udp::socket(*io_,remote_udp_ep_.protocol()));

        work_.reset(new boost::asio::io_service::work(*io_));

         // Create JSON message with the content 
        Json::Value message;   

        message["rxMessageIpAddress"] = local_ip;
        message["infrastructureId"] = infrastructure_id;
        message["rxMessagePort"] = v2x_port;
        message["timeSyncPort"] = time_sync_port;
        
        message["location"]["latitude"] = location.Latitude;
        message["location"]["longitude"] = location.Longitude;
        message["location"]["elevation"] = location.Elevation;
 
        try
        {
            Json::StyledWriter writer;
            std::string message_str = writer.write(message);
            udp_out_socket_->send_to(boost::asio::buffer(message_str), remote_udp_ep_);
        }
        catch(boost::system::system_error error_code)
        {
            PLOG(logERROR) << "Encountered runtime error when executing handshake: " << error_code.what() << std::endl;
        }
        catch(...)
        {
            PLOG(logERROR) << "Encountered runtime error when executing handshake: boost::asio::error::fault" << std::endl;
        }

        return true;
    }

    bool CDASimConnection::setup_udp_connection(const std::string &simulation_ip, const std::string &local_ip,  const uint time_sync_port, 
                                const uint v2x_port, const uint simulation_v2x_port) {
        try {
            // Iniitialize CARMA Simulation UDP Server and Client to foward V2X messages between CARMA simulation 
            // and CARMA Simulation Infrastructure Adapter.
            carma_simulation_listener = std::make_shared<UdpServer>( local_ip, v2x_port );
            carma_simulation_publisher = std::make_shared<UdpClient>( simulation_ip, simulation_v2x_port);
            // Initialize V2X-Hub UDP Server and Client to foward V2X messages between CARMA Simulation Infrastructure 
            // Adapter and V2X-Hub.
            // TODO: Using TMX Utils get immediate forward port
            // TODO: Replace 0 with immediate forward port
            immediate_forward_listener = std::make_shared<UdpServer>( local_ip, 0 );
            // TODO: Using TMX Utils get message receiver port
            // TODO: Replace 0 with message receiver port
            message_receiver_publisher = std::make_shared<UdpClient>( local_ip, 0);
            // Initialize UDP Server for listening for incoming CARMA-Simulation time synchronization.
            time_sync_listener = std::make_shared<UdpServer>(local_ip,time_sync_port);
        }
        catch (const UdpClientRuntimeError &e) {
            PLOG(logERROR) << "Encountered UDPClient Runtime error during UDP connection initialization : " << e.what() << std::endl;
            return false;
        }
        catch (const UdpServerRuntimeError &e) {
            PLOG(logERROR) << "Encountered UDPServer Runtime error during UDP connection initialization : " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    tmx::messages::TimeSyncMessage CDASimConnection::consume_time_sync_message() const{
        if (time_sync_listener) {
            std::string str_msg = consume_server_message(time_sync_listener);
            tmx::messages::TimeSyncMessage msg;
            // TODO: Convert str_msg to TimeSyncMessage
            return msg;
        }
        else {
            throw std::runtime_error("Time Sync UDP Server is not initialized");
        }
    }

    std::string CDASimConnection::consume_server_message( const std::shared_ptr<UdpServer> _server) const {
        std::unique_ptr<char> msg = std::make_unique<char>();
        int num_of_bytes = _server->TimedReceive(msg.get(),1000, 5);
        if (num_of_bytes > 0 ) {
            std::string ret(msg.get());
            PLOG(logDEBUG) << "Message Received : " << ret << std::endl;
            return ret;
        }
        else if ( num_of_bytes == 0 ) {
            throw UdpServerRuntimeError("Received empty message!");
        }
        else {
            throw UdpServerRuntimeError("Listen timed out after 5 ms!");
        }
    }

    std::string CDASimConnection::consume_v2x_message_from_simulation() const {
        if ( carma_simulation_listener) {
            std::string msg = consume_server_message( carma_simulation_listener );
            return msg;
        }
        else {
            throw std::runtime_error("CARMA Simulation UDP Server is not initialized!");
        }
    }

    std::string CDASimConnection::consume_v2x_message_from_v2xhub() const {
        if ( immediate_forward_listener) {
            std::string msg = consume_server_message( immediate_forward_listener );
            return msg;
        }
        else {
            throw std::runtime_error("Immediate Forward UDP Server is not initialized!");
        }
    }

    void CDASimConnection::forward_time_sync_message(const tmx::messages::TimeSyncMessage &msg) const {
        //TODO: Forward message on TMX bus and to Kafka 
    }

    void CDASimConnection::forward_message( const std::string &msg, const std::shared_ptr<UdpClient> _client ) const {
        if ( !msg.empty() && _client) {
            int ret =_client->Send(msg);
            if ( ret < 0) {
                throw UdpClientRuntimeError(("Failed to send message " + msg + " to " + _client->GetAddress() + 
                            ":" + std::to_string(_client->GetPort()) + " with error code : " + std::to_string(errno)).c_str());
            }
        }
    }

    void CDASimConnection::forward_v2x_message_to_simulation(const std::string &msg) const {
        forward_message( msg, carma_simulation_publisher );
    }

    void CDASimConnection::forward_v2x_message_to_v2xhub(const std::string &msg) const {
        forward_message( msg , message_receiver_publisher );
    }

}