#include "UDPMessageForwarder.h"


namespace ODEForwardPlugin
{
    void UDPMessageForwarder::sendMessage(UDPMessageType messageType, const std::string& message){
        try{
            _udpClientsMap.at(messageType)->Send(message);
        }catch(const std::out_of_range& oor){
            throw TmxException("UDP Client not found for message type");
        }
    }

    void UDPMessageForwarder::attachUdpClient(UDPMessageType messageType, std::shared_ptr<UdpClient> udpClient){
        //If udp client exists for a message type, remove it first
        if(_udpClientsMap.find(messageType) != _udpClientsMap.end()){
            _udpClientsMap.erase(messageType);
        }
        _udpClientsMap.insert(std::make_pair(messageType, udpClient));
    }

    std::vector<UDPMessageType> UDPMessageForwarder::getAllUdpClientMessageTypes(){
        std::vector<UDPMessageType> udpClientTypes;
        for(auto& udpClient : _udpClientsMap){
            //Push the udp message type to the vector
            udpClientTypes.push_back(udpClient.first);
        }
        return udpClientTypes;
    }

    std::shared_ptr<UdpClient> UDPMessageForwarder::getUdpClient(UDPMessageType messageType){
        return _udpClientsMap[messageType];
    }
}