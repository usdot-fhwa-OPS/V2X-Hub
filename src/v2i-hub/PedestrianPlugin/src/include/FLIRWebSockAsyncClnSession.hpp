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
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <queue>
#include <ctime>
#include <cmath>
#include <atomic>

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
        bool generatePSM_;
        bool generateSDSM_;
        bool generateTIM_;
        /***
         * Each websocket is connected to one camera responsible for one region/view at an intersection.
         * This isPedestrainPresent indicator signify whether there is a pedestrain in that region or not.
        ***/
        std::atomic<bool> isPedestrainPresent_;
        std::string pedPresenceTrackingReq = std::string("{\"messageType\":\"Subscription\", \"subscription\":{ \"type\":\"Data\", \"action\":\"Subscribe\", \"inclusions\":[{\"type\":\"PedestrianPresenceTracking\"}]}}");
        float cameraRotation_;
        std::string cameraViewName_;
        std::string psmxml = "";
        std::string sdsmxml = "";
        std::string timxml = "";
        std::queue<std::string> msgQueue;

        std::mutex _msgLock;
        int msgCount = 0;
        std::atomic<int> moy_;
        std::atomic<int> startYear_;
    public:

    // Resolver and socket require an io_context
    explicit FLIRWebSockAsyncClnSession(net::io_context& ioc)
        : resolver_(net::make_strand(ioc)), ws_(net::make_strand(ioc)), isPedestrainPresent_(false), moy_(0), startYear_(0){};

    /**
     * @brief Calculates the current minute of the year to be included in the Traveler Information Message (TIM).
     * @param year Current year
     * @param month Current month
     * @param day Current day
     * @param hour Current hour
     * @param minute Current minute
     * @param second Current second
     */
    int calculateMinuteOfYear(int year, int month, int day, int hour, int minute, int second) const;

    /**
     * @brief Reports a failure with any of the websocket functions below
     * @param ec the error code for the specific function 
     * @param what description of the error 
     */
    void fail(beast::error_code ec, char const* what) const;       

    /**
     * @brief Start the asynchronous web socket connection to the camera. Each function will call the
     * function below it.
     * @param host ip address of camera to connect to
     * @param port port to connect to
     * @param cameraRotation calculated camera rotation
     */
    void run(char const* host, char const* port, float cameraRotation,const char* cameraViewName, char const* hostString, bool generatePSM, bool generateSDSM, bool generateTIM);
    
    /**
     * @brief Lookup the domain name of the IP address from run function.
     * @param ec error code containing information describing resolve issue
     * @param results result of domain name lookup
     */
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    
    /**
     * @brief Configures websocket settings and initiates handshake
     * @param ec error code containing information describing connection issue 
     */
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

    /**
     * @brief Performs the websocket handshake and calls write function
     * @param ec error code containing information describing handshake issue
     */
    void on_handshake(beast::error_code ec);

    /**
     * @brief Sends the subscription request json to the camera and calls read function for camera response
     * @param ec error code containing information describing issue with json send
     * @param bytes_transferred the bytes of the json
     */
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    /**
     * @brief Used to read in all messages from the camera and parse out desired fields
     * @param ec error code containing information describing issue with reading camera data
     * @param bytes_transferred the bytes of the received camera data
     */
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    
    /**
     * @brief Closes the websocket connection to the camera
     * @param ec error code containing information describing issue with closing websocket
     */
    void on_close(beast::error_code ec);

    /**
     * @brief Get method for queue containing message(s) for all tracked pedestrians. Copies the queue into
     * a temporary queue and returns temporary queue. Clears the original queue.
     * @return The message queue.
     */
    std::queue<std::string> getMsgQueue();

    /**
     * @brief Parses the datetime string that the camera returns into a vector containing each component
     * @param datetime string from camera 
     * @return Vector with all components 
     */
    std::vector<int> timeStringParser(std::string dateTimeStr) const;

    /**
     * @brief Handles messages of type "Subscription" received from the FLIR camera
     * @param pr Property tree containing the parsed JSON message
     */
    void handleSubscriptionMessage(const pt::ptree& pr) const;

    /**
     * @brief Handles messages of type "Data" received from the FLIR camera
     * @param pr Property tree containing the parsed JSON message
     */
    void handleDataMessage(const pt::ptree& pr);

    /**
     * @brief Processes pedestrian presence tracking data, generates appropriate XML messages (PSM, SDSM, TIM), 
     * and pushes them to a message queue
     * @param pr Property tree containing the parsed JSON data
     * @param time Timestamp string associated with received data
     */
    void processPedestrianData(const pt::ptree& pr, const std::string& time);
    /**
     * @brief Get the latest status whether a pedestrain exists at a crosswalk
     */
    bool isPedestrainPresent() const;
    /**
     * @brief Update the pedestrain status to indicator whether there is a pedestrain at a crosswalk
     * @param isPresent latest pedestian status to update
     */
    void setPedestrainPresence(bool isPresent);

    /**
     * @brief Get the minute of the year
     */
    int getMoy() const;
    /**
     * @brief Get the start year of the current year
     */
    int getStartYear() const;
    };
};
