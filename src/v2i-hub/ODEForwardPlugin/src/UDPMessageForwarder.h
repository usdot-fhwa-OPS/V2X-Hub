#pragma once
#include <UdpClient.h>
#include <unordered_map>
#include <tmx/TmxException.hpp>
#include <vector>
#include <bitset>

using tmx::utils::UdpClient;
using tmx::TmxException;

namespace ODEForwardPlugin
{
    /**
     * @brief UDP message type: TIM, MAP, SPAT, BSM
     */
    enum class UDPMessageType{
        TIM,
        MAP,
        SPAT,
        BSM
    };

    class UDPMessageForwarder
    {
    private:
        //Map for udp client and udp message type
        std::unordered_map<UDPMessageType, std::shared_ptr<UdpClient>> _udpClientsMap;
    public:
        explicit UDPMessageForwarder() = default;
        ~UDPMessageForwarder() = default;
        /**
         * @brief Send difference type of messages to via their corresponding udp client
         * @param messageType The message type to be sent
         * @param message The message to be sent
         */
        void sendMessage(UDPMessageType messageType, const std::string& message);
        /***
         * @brief Attach the corresponding udp client to the message type
         * @param messageType Given message type
         * @param udpClient The udp client to be attached
         */
        void attachUdpClient(UDPMessageType messageType, std::shared_ptr<UdpClient> udpClient);

        /**
         * @brief Get all udp client message types
         * @return A vector of all udp client message types
         */
        std::vector<UDPMessageType> getAllUdpClientMessageTypes() const;

        /**
         * @brief Get the udp client for a given message type
         * @return The udp client for the given message type
         */
        std::shared_ptr<UdpClient> getUdpClient(UDPMessageType messageType) const;
        /**
         * @brief Convert hex string to bytes string
         * @param hex The hex string to be converted
         * @return The bytes string
         */
        std::string hexToBytes(const std::string& hex) const;
        /**
         * @brief Convert UDPMessageType enum to string
         * @param UDPMessageType 
         * @return String representation of the UDPMessageType
         */
        std::string toString(UDPMessageType messageType) const;

    };    
} 
