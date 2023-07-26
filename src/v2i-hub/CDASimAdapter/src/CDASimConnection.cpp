#include "include/CDASimConnection.hpp"


using namespace tmx::utils;

namespace CDASimAdapter{ 
    CDASimConnection::CDASimConnection(const std::string &simulation_ip, const std::string &infrastructure_id, const uint simulation_registration_port, const uint sim_v2x_port,
                                                        const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                                        const Point &location, const std::string &sensor_json_file_path) : 
                                                        _simulation_ip(simulation_ip), _infrastructure_id(infrastructure_id), _simulation_registration_port(simulation_registration_port),
                                                        _simulation_v2x_port(sim_v2x_port), _local_ip(local_ip), _time_sync_port(time_sync_port), _v2x_port(v2x_port),
                                                        _location(location) ,_sensor_json_file_path(sensor_json_file_path) {
        PLOG(logDEBUG) << "CARMA-Simulation connection initialized." << std::endl;                                                     
    } 

 

    bool CDASimConnection::is_connected() const {
        return _connected;
    }

    bool CDASimConnection::connect() {
        //Read local sensor file and populate the sensors JSON
        populate_sensors_with_file(_sensor_json_file_path, _sensors_json_v);
        if(_sensors_json_v.empty())
        {
            PLOG(logWARNING) << "Sensors JSON is empty!" << std::endl;
        }     
        if (!carma_simulation_handshake(_simulation_ip, _infrastructure_id, _simulation_registration_port, _local_ip, _time_sync_port, _v2x_port, _location, _sensors_json_v)) {
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

    std::string CDASimConnection::get_handshake_json(const std::string &infrastructure_id, const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                const Point &location, const Json::Value& sensors_json_v) const

    {
        Json::Value message;   
        std::string message_str = "";
        
        message["rxMessageIpAddress"] = local_ip;
        message["infrastructureId"] = infrastructure_id;
        message["rxMessagePort"] = v2x_port;
        message["timeSyncPort"] = time_sync_port;
        message["location"]["x"] = location.X;
        message["location"]["y"] = location.Y;
        message["location"]["z"] = location.Z;
        message["sensors"] = sensors_json_v;
        Json::StyledWriter writer;
        message_str = writer.write(message);
        return message_str;
    }

    bool CDASimConnection::carma_simulation_handshake(const std::string &simulation_ip, const std::string &infrastructure_id, const uint simulation_registration_port, 
                                const std::string &local_ip,  const uint time_sync_port, const uint v2x_port,
                                const Point &location, const Json::Value& sensors_json_v ) 
    {
        // Create JSON message with the content 
        std::string payload = "";
    
        payload = get_handshake_json(infrastructure_id, local_ip, time_sync_port, v2x_port, location, sensors_json_v);
    
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

    std::string CDASimConnection::consume_hex_server_message( const std::shared_ptr<UdpServer> _server) const {
        std::vector<char> msg(4000);
        int num_of_bytes = _server->TimedReceive(msg.data(),4000, 5);
        if (num_of_bytes > 0 ) {
            msg.resize(num_of_bytes);
            std::string ret = "";
            for (char c: msg)
            {
                ret += c;
            }
            PLOG(logDEBUG) << "UDP Server message received size and payload: " << ret.size() <<" and "<< ret << std::endl;
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
            std::string msg = consume_hex_server_message( carma_simulation_listener );
            return msg;
        }
        else {
            throw std::runtime_error("CARMA Simulation UDP Server is not initialized!");
        }
        return "";

    }

    std::string CDASimConnection::consume_v2x_message_from_v2xhub() const {
        if ( immediate_forward_listener) {
            std::string msg = consume_hex_server_message( immediate_forward_listener );
            return msg;
        }
        else {
            throw std::runtime_error("Immediate Forward UDP Server is not initialized!");
        }
        return "";

    }



    void CDASimConnection::forward_message( const std::string &msg, const std::shared_ptr<UdpClient> _client ) const {
        if ( !msg.empty() && _client) {
            PLOG(logDEBUG) << "Sending UDP msg " << msg << " to host " << _client->GetAddress() 
                << ":" << _client->GetPort() << "." << std::endl;
            int ret =_client->Send(msg);
            if ( ret < 0) {
                throw UdpClientRuntimeError(("Failed to send message " + msg + " to " + _client->GetAddress() + 
                            ":" + std::to_string(_client->GetPort()) + " with error code : " + std::to_string(errno)).c_str());
            }
        }
        else if ( msg.empty() ) {
            PLOG(logWARNING) << "Unable to send empty message to " << _client->GetAddress() << ":" << _client->GetPort() << "." << std::endl;
        }
        else {
            PLOG(logWARNING) << "Unable to send message from uninitialized client." << std::endl;
        }
    }

    void CDASimConnection::forward_v2x_message_to_simulation(const std::string &msg) const {
        forward_message( msg, carma_simulation_publisher );
    }

    void CDASimConnection::forward_v2x_message_to_v2xhub(const std::string &msg) const {
        forward_message( msg , message_receiver_publisher );
    }

    void CDASimConnection::populate_sensors_with_file(const std::string& file_path, Json::Value& sensors_json_v){
        //Read file from disk
        std::ifstream in_strm;
        in_strm.open(file_path, std::ifstream::binary);
        if(!in_strm.is_open())
        {
            PLOG(logERROR) << "File cannot be opened. File path: " << file_path <<std::endl;
            return;
        }
        std::string line;
        std::stringstream ss;
        while (std::getline(in_strm, line)) {
            ss << line;
        }
        in_strm.close();

       populate_json_with_string(ss.str(), sensors_json_v);
    }

    void CDASimConnection::populate_json_with_string(const std::string& json_str, Json::Value& json_v){
        //Update JSON value with information from file
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        JSONCPP_STRING err;
        if(!reader->parse(json_str.c_str(), json_str.c_str() + json_str.length(), &json_v, &err))
        {
            PLOG(logERROR) << "Error parsing sensors from string: " << json_str << std::endl;
            return;
        }
    }

    Json::Value CDASimConnection::get_sensor_by_id(std::string &sensor_id)
    {
        Json::Value result;
        if(_sensors_json_v.empty())
        {
            PLOG(logERROR) << "CDASimAdapter stored sensors are empty." << std::endl;
            return result;
        }

        if(_sensors_json_v.isArray())
        {
           std::for_each( _sensors_json_v.begin(), _sensors_json_v.end(), [&result, &sensor_id](auto& item){
                if(item["id"] == sensor_id)
                {
                    result = item;
                }
           });
        }
        else if(_sensors_json_v.isObject())
        {
            if(_sensors_json_v["id"] == sensor_id)
            {
                result = _sensors_json_v;
            }
        }

        if(result.empty())
        {
            PLOG(logERROR) << "CDASimAdapter stored sensors do not have sensor with id = " << sensor_id << std::endl;
        }
        return result;
    }
}
