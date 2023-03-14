#include "include/CARMASimulationConnection.hpp"


using namespace tmx::utils;

namespace CARMASimulationAdapter{ 
    CARMASimulationConnection::CARMASimulationConnection(const std::string &simulation_ip, const uint simulation_registration_port, 
                                                        const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                                        const tmx::utils::WGS84Point &location, 
                                                        std::shared_ptr<kafka_clients::kafka_producer_worker> time_producer) : 
                                                        _simulation_ip(simulation_ip) , _simulation_registration_port(simulation_registration_port),
                                                        _local_ip(local_ip), _time_sync_port(time_sync_port), _v2x_port(v2x_port),
                                                        _location(location), _time_producer(time_producer)  {
        PLOG(logDEBUG) << "CARMA-Simulation connection initialized." << std::endl;                                                     
    } 

    CARMASimulationConnection::~CARMASimulationConnection() {
        PLOG(logDEBUG) << "CARMA-Simulation connection destroyed." << std::endl; 
    }

    bool CARMASimulationConnection::is_connected() const {
        return _connected;
    }

    bool CARMASimulationConnection::connect() {
        if (!carma_simulation_handshake(_simulation_ip, _simulation_registration_port, _local_ip, _time_sync_port, _v2x_port, _location)) {
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
    bool CARMASimulationConnection::carma_simulation_handshake(const std::string &simulation_ip, const uint simulation_registration_port, 
                                const std::string &local_ip,  const uint time_sync_port, const uint v2x_port, 
                                const tmx::utils::WGS84Point &location) {
        // TODO
        return false;                                
    }

    bool CARMASimulationConnection::setup_udp_connection(const std::string &simulation_ip, const std::string &local_ip,  const uint time_sync_port, 
                                const uint v2x_port, const uint simulation_v2x_port) {
        // TODO
        return false;
    }
}