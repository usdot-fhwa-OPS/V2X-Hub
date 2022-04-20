#include "include/FLIRWebSockAsyncClnSession.hpp"
#include <PluginLog.h>
#include <TmxLog.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/buffers_iterator.hpp>

namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>

//using namespace tmx::messages;
using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace PedestrianPlugin
{

 // Start the asynchronous operation
    void
    FLIRWebSockAsyncClnSession::run(
        char const* host,
        char const* port)
    {
        
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::run " << std::endl;
	    // Save these for later
        host_ = host;
        PLOG(logDEBUG) << "Host: "<< host <<" ; port: "<< port << std::endl;       

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_resolve,
                shared_from_this()));
    }

    void
    FLIRWebSockAsyncClnSession::on_resolve(
        beast::error_code ec,
        tcp::resolver::results_type results)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_resolve " << std::endl;
        if(ec)
            return fail(ec, "resolve");             

        // Set the timeout for the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_connect,
                shared_from_this()));
    }

    void
    FLIRWebSockAsyncClnSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_connect " << std::endl;
        if(ec)
            return fail(ec, "connect");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();      

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-async");
            }));

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        // host_ += ':' + std::to_string(ep.port());

        PLOG(logDEBUG) << "host_: " << host_ << std::endl;

        // Perform the websocket handshake
        ws_.async_handshake(host_, "/api/subscriptions",
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_handshake,
                shared_from_this()));
    }

    void
    FLIRWebSockAsyncClnSession::on_handshake(beast::error_code ec)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_handshake " << std::endl;
        if(ec)
            return fail(ec, "handshake");
        
        // Send the message
        ws_.async_write(
            net::buffer(pedPresenceTrackingReq),
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_write,
                shared_from_this()));
    }

    void
    FLIRWebSockAsyncClnSession::on_write(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_write " << std::endl;
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        
        // Read a message into our buffer, 
        // this should be the response to the subscribe message
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_read,
                shared_from_this()));
    }

    void
    FLIRWebSockAsyncClnSession::on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_read " << std::endl;
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        // parse the buffer
        std::stringstream ss; 
	    ptree pr; 
	    std:string text(beast::buffers_to_string(buffer_.data()));
        ss<<text; 

        PLOG(logDEBUG) << "Received:  " << text.c_str() << std::endl;

        try
        {
	        read_json(ss, pr);
        }
        catch(const ptree_error &e)
        {
            PLOG(logERROR) << "Error converting json to p tree:  " << e.what() << std::endl;
        }

        //Example FLIR subscription response json:
        //Received:  {"messageType": "Subscription", "subscription": {"returnValue": "OK", "type": "Data"}}

        std::string messageType = pr.get_child("messageType").get_value<string>();
        if (messageType.compare("Subscription") == 0)
        {
            std::string subscrStatus = pr.get_child("subscription").get_child("returnValue").get_value<string>();
            PLOG(logDEBUG) << "Ped presence data subscription status: " << subscrStatus << std::endl;
        
        }
        //Example received pedestrian tracking data       
        //{"dataNumber": "473085", "messageType": "Data", "time": "2022-04-20T15:25:51.001-04:00", 
        //"track": [{"angle": "263.00000000", "class": "Pedestrian", "iD": "15968646", "latitude": "38.95499217", 
        //"longitude": "-77.14920953", "speed": "1.41873741", "x": "0.09458912", "y": "14.80903757"}], "type": "PedestrianPresenceTracking"}
        else if (messageType.compare("Data") == 0)
        {
            std::string time = pr.get_child("time").get_value<string>(); //TODO: convert to timestamp
            std::string type =  pr.get_child("type").get_value<string>();
            if (type.compare("PedestrianPresenceTracking") == 0)
            {
                try
                {
                    ptree track = pr.get_child("track");
                    int angle = track.get_child("angle").get_value<int>(); //TODO: convert
                    int id = track.get_child("iD").get_value<int>(); 
                    double latitude = track.get_child("latitude").get_value<double>();
                    double longitude = track.get_child("longitude").get_value<double>();
                    float speed = track.get_child("longitude").get_value<float>();
                    //TODO: print to PSM xml and call BroadcastPsm
                }
                catch(const ptree_error &e))
                {
                    PLOG(logERROR) << "Error with track data:  " << e.what() << std::endl;
                }

            }            
        }
        else
        {
            PLOG(logDEBUG) << "Received unknown message: " << text.c_str() << std::endl;
        }

        // need to clear the buffer after reading the message
        buffer_.consume(buffer_.size());  

        // this will read data subscribed to
        ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &FLIRWebSockAsyncClnSession::on_read,
            shared_from_this()));
    }

    void
    FLIRWebSockAsyncClnSession::on_close(beast::error_code ec)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_close " << std::endl;
        if(ec)
            return fail(ec, "close");

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }
}