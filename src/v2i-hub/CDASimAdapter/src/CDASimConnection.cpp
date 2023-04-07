#include "include/CDASimConnection.hpp"


using namespace tmx::utils;

namespace CDASimAdapter{ 
    CDASimConnection::CDASimConnection(const std::string &simulation_ip, const uint infrastructure_id, const uint simulation_registration_port, const uint sim_v2x_port,
                                                        const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                                        const WGS84Point &location) : 
                                                        _simulation_ip(simulation_ip), _infrastructure_id(infrastructure_id), _simulation_registration_port(simulation_registration_port),
                                                        _simulation_v2x_port(sim_v2x_port), _local_ip(local_ip), _time_sync_port(time_sync_port), _v2x_port(v2x_port),
                                                        _location(location)  {
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
        _connected =true;
        PLOG(logINFO) << "CARMA-Simulation connection is successful!" << std::endl;
        return _connected;
    }

    std::string CDASimConnection::get_handshake_json(const uint infrastructure_id, const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                const WGS84Point &location)

    {
        Json::Value message;   
        std::string message_str = "";
        
        message["rxMessageIpAddress"] = local_ip;
        message["infrastructureId"] = infrastructure_id;
        message["rxMessagePort"] = v2x_port;
        message["timeSyncPort"] = time_sync_port;
        message["location"]["latitude"] = location.Latitude;
        message["location"]["longitude"] = location.Longitude;
        message["location"]["elevation"] = location.Elevation;
        Json::StyledWriter writer;
        message_str = writer.write(message);
        return message_str;
    }

    bool CDASimConnection::carma_simulation_handshake(const std::string &simulation_ip, const uint infrastructure_id, const uint simulation_registration_port, 
                                const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                const WGS84Point &location) 
    {
        // Create JSON message with the content 
        std::string payload = "";
    
        payload = get_handshake_json(infrastructure_id, local_ip, time_sync_port, v2x_port, location);
    
        try
        {
            carma_simulation_registration_publisher = std::make_shared<UdpClient>( simulation_ip, simulation_registration_port);
            forward_message(payload, carma_simulation_registration_publisher);
        }
        catch(const std::exception &e)
        {
            PLOG(logERROR) << "Encountered runtime error when executing handshake:" << e.what() << std::endl;
            return false;
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
            immediate_forward_listener = std::make_shared<UdpServer>( local_ip, 5678);
            // TODO: Using TMX Utils get message receiver port
            // TODO: Replace 0 with message receiver port
            message_receiver_publisher = std::make_shared<UdpClient>( local_ip, 8765);
            // Initialize UDP Server for listening for incoming CARMA-Simulation time synchronization.
            PLOG(logDEBUG) << "Creating UDPServer Time Sync Messages" << local_ip << ":" << std::to_string(time_sync_port) << "\n" 
                            << "Creating UDPServer for CDA V2X message forwarding" << local_ip << ":" << std::to_string(v2x_port) << "\n" 
                            << "Creating UDPClient for CDA V2X message forwarding " << simulation_ip << ":" << std::to_string(simulation_v2x_port) << "\n"
                            << "Creating UDPServer for Immediate Forward " << local_ip << ":" << std::to_string(5678) << "\n"
                            << "Creating UDPClient for Message Receiver " << local_ip << ":" << std::to_string(8765) << std::endl;
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
        PLOG(logINFO) << "UDP Connection to CDASim setup Successfully!" << std::endl;
        return true;
    }

    tmx::messages::TimeSyncMessage CDASimConnection::consume_time_sync_message() const{
        tmx::messages::TimeSyncMessage msg;
        msg.clear();
        if (time_sync_listener) {
            std::string str_msg = consume_server_message(time_sync_listener);
            msg.set_contents( str_msg );
        }
        else {
            throw std::runtime_error("Time Sync UDP Server is not initialized");
        }
        return msg;

    }

    std::string CDASimConnection::consume_server_message( const std::shared_ptr<UdpServer> _server) const {
        std::vector<char> msg(4000);
        int num_of_bytes = _server->TimedReceive(msg.data(),4000, 5);
        if (num_of_bytes > 0 ) {
            msg.resize(num_of_bytes);
            std::string ret(msg.data());
            PLOG(logDEBUG) << "UDP Server message received : " << ret << " of size " << num_of_bytes << std::endl;
            return ret;
        }
        else if ( num_of_bytes == 0 ) {
            throw UdpServerRuntimeError("Received empty message!");
        }
        else {
            throw UdpServerRuntimeError("Listen timed out after 5 ms!");
        }
        return "";
    }

    std::string CDASimConnection::consume_v2x_message_from_simulation() const {
        if ( carma_simulation_listener) {
            std::string msg = consume_server_message( carma_simulation_listener );
            return msg;
        }
        else {
            throw std::runtime_error("CARMA Simulation UDP Server is not initialized!");
        }
        return "";

    }

    std::string CDASimConnection::consume_v2x_message_from_v2xhub() const {
        if ( immediate_forward_listener) {
            std::string msg = consume_server_message( immediate_forward_listener );
            return msg;
        }
        else {
            throw std::runtime_error("Immediate Forward UDP Server is not initialized!");
        }
        return "";

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