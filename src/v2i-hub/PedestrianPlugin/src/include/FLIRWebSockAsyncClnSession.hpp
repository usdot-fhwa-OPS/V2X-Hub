#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <PluginLog.h>
#include <TmxLog.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <queue>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace pt = boost::property_tree;    // from <boost/property_tree/ptree.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

namespace PedestrianPlugin
{
    // Sends a WebSocket message and prints the response
    class FLIRWebSockAsyncClnSession : public std::enable_shared_from_this<FLIRWebSockAsyncClnSession>
    {
        tcp::resolver resolver_;
        websocket::stream<beast::tcp_stream> ws_;
        beast::flat_buffer buffer_;
        std::string host_;
        std::string hostString_;
        std::string pedPresenceTrackingReq = std::string("{\"messageType\":\"Subscription\", \"subscription\":{ \"type\":\"Data\", \"action\":\"Subscribe\", \"inclusions\":[{\"type\":\"PedestrianPresenceTracking\"}]}}");
        float cameraRotation_;
        std::string psmxml = "";
        std::queue<std::string> psmQueue;

        std::mutex _psmLock;
        int msgCount = 0;
    public:

    // Resolver and socket require an io_context
    explicit     
    FLIRWebSockAsyncClnSession(net::io_context& ioc)
        : resolver_(net::make_strand(ioc))
        , ws_(net::make_strand(ioc))
    {

    };

    /**
     * @brief Reports a failure with any of the websocket functions below
     * 
     * @param: the error code for the specific function 
     * @param: description of the error 
     */
    void
    fail(beast::error_code ec, char const* what) const;       

    /**
     * @brief Start the asynchronous web socket connection to the camera. Each function will call the
     * function below it.
     * 
     * @param: ip address of camera to connect to
     * @param: port to connect to
     * @param: calculated camera rotation
     */
    void
    run(
        char const* host,
        char const* port,
        float cameraRotation, 
        char const* hostString);
    
    /**
     * @brief Lookup the domain name of the IP address from run function.
     * 
     * @param: error code containing information describing resolve issue
     * @param: result of domain name lookup
     */
    void
    on_resolve(
        beast::error_code ec,
        tcp::resolver::results_type results);
    
    /**
     * @brief Configures websocket settings and initiates handshake
     * 
     * @param: error code containing information describing connection issue 
     */
    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

    /**
     * @brief Performs the websocket handshake and calls write function
     * 
     * @param: error code containing information describing handshake issue
     */
    void
    on_handshake(beast::error_code ec);

    /**
     * @brief Sends the subscription request json to the camera and calls read function for camera response
     * 
     * @param: error code containing information describing issue with json send
     * @param: the bytes of the json
     */
    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred);

    /**
     * @brief Used to read in all messages from the camera and parse out desired fields
     * 
     * @param: error code containing information describing issue with reading camera data
     * @param: the bytes of the received camera data
     */
    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred);
    
    /**
     * @brief Closes the websocket connection to the camera
     * 
     * @param: error code containing information describing issue with closing websocket
     */
    void
    on_close(beast::error_code ec);

    /**
     * @brief Get method for queue containing psm for all tracked pedestrians. Copies the queue into
     * a temporary queue and returns temporary queue. Clears the original queue.
     * 
     * @return std::queue the psm queue
     */
    std::queue<std::string> getPSMQueue();


    /**
     * @brief Parses the datetime string that the camera returns into a vector containing each component
     * 
     * @param: datetime string from camera 
     * @return: vector with all components 
     */
    std::vector<int> timeStringParser(std::string dateTimeStr) const;        
    };  


};

