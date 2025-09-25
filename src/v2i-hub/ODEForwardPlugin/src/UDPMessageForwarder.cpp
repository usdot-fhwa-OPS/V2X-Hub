#include "UDPMessageForwarder.h"


namespace ODEForwardPlugin
{
    void UDPMessageForwarder::sendMessage(UDPMessageType messageType, const std::string& message){
        try{
            auto messageBytes = hexToBytes(message);
            _udpClientsMap.at(messageType)->Send(messageBytes);
        }catch(const std::out_of_range& ex){
            BOOST_THROW_EXCEPTION(TmxException("UDP Client not found for message type. Error message: " + std::string(ex.what())));
        }catch(const std::invalid_argument& ex){
            BOOST_THROW_EXCEPTION(TmxException("Invalid hex string. Error message: " + std::string(ex.what())));
        }
    }

    void UDPMessageForwarder::attachUdpClient(UDPMessageType messageType, std::shared_ptr<UdpClient> udpClient){
        //If udp client exists for a message type, remove it first
        if(_udpClientsMap.find(messageType) != _udpClientsMap.end()){
            _udpClientsMap.erase(messageType);
        }
        _udpClientsMap.insert(std::make_pair(messageType, udpClient));
    }

    std::vector<UDPMessageType> UDPMessageForwarder::getAllUdpClientMessageTypes() const{
        std::vector<UDPMessageType> udpClientTypes;
        for(const auto& udpClient : _udpClientsMap){
            //Push the udp message type to the vector
            udpClientTypes.push_back(udpClient.first);
        }
        return udpClientTypes;
    }

    std::shared_ptr<UdpClient> UDPMessageForwarder::getUdpClient(UDPMessageType messageType) const{
        try{
            return _udpClientsMap.at(messageType);
        }catch(const std::out_of_range& ex){
            BOOST_THROW_EXCEPTION(TmxException("UDP Client not found for message type. Error message: " + std::string(ex.what())));
        }
    }

    std::string UDPMessageForwarder::hexToBytes(const std::string& hex) const{
        std::vector<unsigned char> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
            bytes.push_back(byte);
        }
        return std::string(bytes.begin(), bytes.end());
    }
}