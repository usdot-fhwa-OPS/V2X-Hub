#include "include/FLIRWebSockAsyncClnSession.hpp"
#include <PluginLog.h>
#include <TmxLog.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/buffers_iterator.hpp>

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
        char const* port,
        float cameraRotation)
    {
        
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::run " << std::endl;
	    // Save these for later
        host_ = host;
        cameraRotation_ = cameraRotation;

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
        std::string time = "";
        std::string type = "";
        float angle = 0;
        float alpha = 0;
        float lat = 0;
        float lon = 0;
        float speed = 0;
        int id = 0;
        // long int x_coord = 0;
        // long int y_coord = 0;
        std::string timeString = "";

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
            time = pr.get_child("time").get_value<string>(); 
            type =  pr.get_child("type").get_value<string>();

            PLOG(logINFO) << "Received " << type << " data at time: " << time << std::endl;

            if (type.compare("PedestrianPresenceTracking") == 0)
            {
                try
                {
                    for (auto it: pr.get_child("track")) 
                    {                        

                        if (!it.second.get_child("angle").data().empty())
                        {
                            angle = std::stof(it.second.get_child("angle").data());
                            //convert camera reference frame angle
                            alpha = cameraRotation_ - angle - 270;

                            while (alpha < 0)
                            {
                                alpha = 360 + alpha;
                            }

                            //divide by 0.0125 for J2735 format
                            alpha /= 0.0125;
                            PLOG(logINFO) << "converted ped angle:  " << alpha << std::endl;
                        }                        
                        if (!it.second.get_child("iD").data().empty())
                        {
                            id = std::stoi(it.second.get_child("iD").data()); 
                            PLOG(logINFO) << "ped iD:  " << id << std::endl;
                        }
                        if (!it.second.get_child("latitude").data().empty())
                        {
                            //converting lat/lon to J2735 lat/lon format
                            lat = std::stof(it.second.get_child("latitude").data()) * 10000000; 
                            PLOG(logINFO) << "ped latitude:  " << lat << std::endl;
                        }
                        if (!it.second.get_child("longitude").data().empty())
                        {
                            //converting lat/lon to J2735 lat/lon format
                            lon = std::stof(it.second.get_child("longitude").data()) * 10000000; 
                            PLOG(logINFO) << "ped longitude:  " << lon << std::endl;
                        }
                        if (!it.second.get_child("speed").data().empty())
                        {
                            //speed from the FLIR camera is reported in m/s, need to convert to units of 0.02 m/s
                            speed = std::stof(it.second.get_child("speed").data()) / 0.02;  
                            PLOG(logINFO) << "ped speed:  " << speed << std::endl;
                        }
                        // if (!it.second.get_child("x").data().empty())
                        // {
                        //     x_coord = std::stod(it.second.get_child("x").data()); 
                        // }
                        // if (!it.second.get_child("y").data().empty())
                        // {
                        //     y_coord = std::stod(it.second.get_child("y").data()); 
                        // }
                        //need to parse out seconds from datetime string
                        int *dateTimeArr = timeStringParser(time);

                        //constructing xml to send to BroadcastPSM function
                        char psm_xml_char[10000]; 
                        sprintf(psm_xml_char,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><PersonalSafetyMessage><basicType><aPEDESTRIAN/></basicType>"
                        "<secMark>%i</secMark><msgCnt>0</msgCnt><id>%i</id><position><lat>%.0f</lat><long>%.0f</long></position><accuracy>"
                        "<semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy>"
                        "<speed>%.0f</speed><heading>%.0f</heading><pathHistory><initialPosition><utcTime><year>%i</year><month>%i</month>"
                        "<day>%i</day><hour>%i</hour><minute>%i</minute><second>%i</second></utcTime>"
                        "<long>0</long><lat>0</lat></initialPosition><crumbData><latOffset>0</latOffset><lonOffset>0</lonOffset>"
                        "<elevationOffset>0</elevationOffset><timeOffset>0</timeOffset></crumbData></pathHistory>"
                        "</PersonalSafetyMessage>", dateTimeArr[5], id, lat, lon, speed, alpha, dateTimeArr[0], dateTimeArr[1], dateTimeArr[2], dateTimeArr[3],
                        dateTimeArr[4], dateTimeArr[5]);


                        string psm_xml_str(psm_xml_char);
                        psmxml = psm_xml_str;
                        
                        PLOG(logDEBUG) << "Sending PSM xml to BroadcastPsm: " << psmxml.c_str() <<endl;

                    }
                }
                catch(const ptree_error &e)
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

    //returns the variable containing the psm xml 
    string FLIRWebSockAsyncClnSession::getPSMXML()
    {
        return psmxml;
    }

    int* FLIRWebSockAsyncClnSession::timeStringParser(string dateTimeStr)
    {
        // string delimiter1 = ".";
        // string delimiter2 = "-";

        // string sec = dateTimeStr.substr(dateTimeStr.find(delimiter1)+1, dateTimeStr.find(delimiter2)-1);
        // sec.erase(0, std::min(sec.find_first_not_of('0'), sec.size()-1));

        // return std::stoi(sec);
        std::string delimiter1 = ".";
        std::string delimiter2 = "-";
        std::string delimiter3 = "T";
        std::string delimiter4 = ":";  

        std::string year = dateTimeStr.substr(0, dateTimeStr.find(delimiter2));
        year.erase(0, std::min(year.find_first_not_of('0'), year.size()-1));
        dateTimeStr.erase(0, dateTimeStr.find(delimiter2) + delimiter2.length());

        std::string month = dateTimeStr.substr(0, dateTimeStr.find(delimiter2));
        month.erase(0, std::min(month.find_first_not_of('0'), month.size()-1));
        dateTimeStr.erase(0, dateTimeStr.find(delimiter2) + delimiter2.length());

        std::string day = dateTimeStr.substr(0, dateTimeStr.find(delimiter3));
        day.erase(0, std::min(day.find_first_not_of('0'), day.size()-1));
        dateTimeStr.erase(0, dateTimeStr.find(delimiter3) + delimiter3.length());

        std::string hour = dateTimeStr.substr(0, dateTimeStr.find(delimiter4));
        hour.erase(0, std::min(hour.find_first_not_of('0'), hour.size()-1));
        dateTimeStr.erase(0, dateTimeStr.find(delimiter4) + delimiter4.length());

        std::string mins = dateTimeStr.substr(0, dateTimeStr.find(delimiter4));
        mins.erase(0, std::min(mins.find_first_not_of('0'), mins.size()-1));
        dateTimeStr.erase(0, dateTimeStr.find(delimiter4) + delimiter4.length());

        std::string sec = dateTimeStr.substr(0, dateTimeStr.find(delimiter1));
        sec.erase(0, std::min(sec.find_first_not_of('0'), sec.size()-1));
        dateTimeStr.erase(0, dateTimeStr.find(delimiter1) + delimiter1.length());

        std::string milliseconds = dateTimeStr.substr(0, dateTimeStr.find(delimiter2));
        milliseconds.erase(0, std::min(milliseconds.find_first_not_of('0'), milliseconds.size()-1));

        static int parsedArr [6] = {std::stoi(year), std::stoi(month), std::stoi(day), std::stoi(hour), std::stoi(mins), std::stoi(milliseconds)};

        return parsedArr;
    }        


}