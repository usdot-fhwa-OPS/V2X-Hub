#include "include/FLIRWebSockAsyncClnSession.hpp"

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace PedestrianPlugin
{
    void
    FLIRWebSockAsyncClnSession::fail(beast::error_code ec, char const* what) const
    {
        PLOG(logDEBUG) << what << ": " << ec.message() << std::endl;
    }

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
	    std::string text(beast::buffers_to_string(buffer_.data()));
        ss<<text; 

        PLOG(logDEBUG) << "Received:  " << text.c_str() << std::endl;

        try
        {
	        pt::read_json(ss, pr);
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
        int alpha = 0;
        std::string lat = "";
        std::string lon = "";
        float speed = 0;        
        std::string timeString = "";
        int id = 0;
        std::string idResult;

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
                            //angle only reported in whole number increments, so int is fine
                            angle = std::stoi(it.second.get_child("angle").data());
                            //convert camera reference frame angle
                            alpha = cameraRotation_ - angle - 270;

                            if (alpha < 0)
                            {
                                alpha = (alpha % 360) + 360;
                            }

                            //divide by 0.0125 for J2735 format
                            alpha /= 0.0125;
                        }                        
                        if (!it.second.get_child("iD").data().empty())
                        {
                            id = std::stoi(it.second.get_child("iD").data());

                            //need to convert the id to 4 octet string
                            std::stringstream idstream;
                            idstream << std::hex << id;
                            std::string result(idstream.str());
                        
                            idResult = result;                            
                            int str_length_diff = 8 - idResult.length();
                            idResult.append(str_length_diff, '0');

                        }
                        if (!it.second.get_child("latitude").data().empty())
                        {
                            //converting lat/lon to J2735 lat/lon format
                            lat = it.second.get_child("latitude").data();
                            lat.erase(std::remove(lat.begin(), lat.end(), '.'), lat.end());
                            lat.pop_back();
                        }
                        if (!it.second.get_child("longitude").data().empty())
                        {
                            //converting lat/lon to J2735 lat/lon format
                            lon = it.second.get_child("longitude").data();
                            lon.erase(std::remove(lon.begin(), lon.end(), '.'), lon.end());
                            lon.pop_back();
                        }
                        if (!it.second.get_child("speed").data().empty())
                        {
                            //speed from the FLIR camera is reported in m/s, need to convert to units of 0.02 m/s
                            speed = std::stof(it.second.get_child("speed").data()) / 0.02;  
                        }                       
                        //need to parse out seconds from datetime string
                        std::vector<int> dateTimeArr = timeStringParser(time);                       

                        msgCount += 1;
                        if (msgCount > 127){
                            msgCount = 0;
                        }

                        PLOG(logINFO) << "Received FLIR camera data at: " << dateTimeArr[0] << "/" << dateTimeArr[1] << "/" << dateTimeArr[2]
                        << " " << dateTimeArr[3] << ":" << dateTimeArr[4] << ":" << dateTimeArr[5] << ":" << dateTimeArr[6] << std::endl;  

                        PLOG(logINFO) << "Received FLIR camera data for pedestrian " << idResult << " at location: (" << lat << ", " << lon <<
                        ")" << ", travelling at speed: " << speed << ", with heading: " << alpha << " degrees" << std::endl;

                        PLOG(logINFO) << "PSM message count: " << msgCount << std::endl;

                        //constructing xml to send to BroadcastPSM function
                        char psm_xml_char[10000]; 
                        snprintf(psm_xml_char,10000,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><PersonalSafetyMessage><basicType><aPEDESTRIAN/></basicType>"
                        "<secMark>%i</secMark><msgCnt>%i</msgCnt><id>%s</id><position><lat>%s</lat><long>%s</long></position><accuracy>"
                        "<semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy>"
                        "<speed>%.0f</speed><heading>%i</heading><pathHistory><initialPosition><utcTime><year>%i</year><month>%i</month>"
                        "<day>%i</day><hour>%i</hour><minute>%i</minute><second>%i</second></utcTime>"
                        "<long>0</long><lat>0</lat></initialPosition><crumbData><PathHistoryPoint><latOffset>0</latOffset>"
                        "<lonOffset>0</lonOffset><elevationOffset>0</elevationOffset><timeOffset>1</timeOffset></PathHistoryPoint></crumbData></pathHistory>"
                        "</PersonalSafetyMessage>", dateTimeArr[6], msgCount, idResult.c_str(), lat.c_str(), lon.c_str(), speed, alpha, dateTimeArr[0], dateTimeArr[1], 
                        dateTimeArr[2], dateTimeArr[3], dateTimeArr[4], dateTimeArr[6]);

                        std::string psm_xml_str(psm_xml_char, sizeof(psm_xml_char) / sizeof(psm_xml_char[0]));

		                std::lock_guard<mutex> lock(_psmLock);
                        psmxml = psm_xml_str;
                        psmQueue.push(psmxml);

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

    std::queue<std::string> FLIRWebSockAsyncClnSession::getPSMQueue()
    {
        std::lock_guard<mutex> lock(_psmLock);

        //pass copy of the queue to Pedestrian Plugin
        std::queue<std::string> queueToPass = psmQueue;

        //empty the queue internally
        std::queue<std::string> empty;
        std::swap(psmQueue, empty);
        
        return queueToPass;

    }

    vector<int> FLIRWebSockAsyncClnSession::timeStringParser(string dateTimeStr) const
    {
        std::string delimiter1 = ".";
        std::string delimiter2 = "-";
        std::string delimiter3 = "T";
        std::string delimiter4 = ":";  
        std::vector<int> parsedArr;

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

        parsedArr.push_back(std::stoi(year));
        parsedArr.push_back(std::stoi(month));
        parsedArr.push_back(std::stoi(day));
        parsedArr.push_back(std::stoi(hour));
        parsedArr.push_back(std::stoi(mins));
        parsedArr.push_back(std::stoi(sec));
        parsedArr.push_back(std::stoi(milliseconds));      

        return parsedArr;
    }       

}