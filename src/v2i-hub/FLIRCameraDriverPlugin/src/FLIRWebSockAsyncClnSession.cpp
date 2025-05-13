#include "include/FLIRWebSockAsyncClnSession.hpp"

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace FLIRCameraDriverPlugin
{   

    void FLIRWebSockAsyncClnSession::fail(beast::error_code ec, const std::string& what)
    {
        isHealthy_.store(false);
        PLOG(logERROR) << what << ": " << ec.message();
    }

    void FLIRWebSockAsyncClnSession::run(const std::string &host, const std::string &port, double cameraRotation, const std::string &cameraViewName, const std::string &hostString)
    {
	    // Save these for later
        host_ = host;
        cameraRotation_ = cameraRotation;
        cameraViewName_ = cameraViewName;
        hostString_ = hostString;

        PLOG(logDEBUG) << "Host: "<< host <<" ; port: "<< port << " ; host string: "<< hostString;

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_resolve,
                shared_from_this()));
    }

    void FLIRWebSockAsyncClnSession::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");
        isHealthy_.store(true);
        // Set the timeout for the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_connect,
                 shared_from_this()));
     }


    void FLIRWebSockAsyncClnSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        if(ec)
            return fail(ec, "connect");
        isHealthy_.store(true);
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

        PLOG(logDEBUG) << "host_: " << host_ << "hostString_: " << hostString_;

        // Perform the websocket handshake
        ws_.async_handshake(host_, hostString_,
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_handshake,
                shared_from_this()));
    }

    void FLIRWebSockAsyncClnSession::on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");
        isHealthy_.store(true);
        // Send the message
        ws_.async_write(
            net::buffer(pedPresenceTrackingReq),
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_write,
                shared_from_this()));
    }

    void FLIRWebSockAsyncClnSession::on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        isHealthy_.store(true);
        // Read a message into our buffer,
        // this should be the response to the subscribe message
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &FLIRWebSockAsyncClnSession::on_read,
                shared_from_this()));
    }

    void FLIRWebSockAsyncClnSession::on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");
        isHealthy_.store(true);
        // parse the buffer
        std::stringstream ss;
	    ptree pr;
	    std::string text(beast::buffers_to_string(buffer_.data()));
        ss<<text;

        PLOG(logDEBUG) << "Received: " << text;

        try {
	        pt::read_json(ss, pr);
        } catch(const ptree_error &e) {
            PLOG(logERROR) << "Error converting json to p tree: " << e.what();
        }

        std::string messageType = pr.get_child("messageType").get_value<string>();

        if (messageType.compare("Subscription") == 0)
        {
            handleSubscriptionMessage(pr);
        }
        else if (messageType.compare("Data") == 0)
        {
            handleDataMessage(pr);
        }
        else
        {
            PLOG(logDEBUG) << "Received unknown message: " << text.c_str();
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

    void FLIRWebSockAsyncClnSession::handleSubscriptionMessage(const pt::ptree& pr) const
    {
        /* Example FLIR subscription response json:
        Received:  {"messageType": "Subscription", "subscription": {"returnValue": "OK", "type": "Data"}}
        */

        std::string subscrStatus = pr.get_child("subscription").get_child("returnValue").get_value<std::string>();
        PLOG(logDEBUG) << "Ped presence data subscription status: " << subscrStatus;
    }

    void FLIRWebSockAsyncClnSession::handleDataMessage(const pt::ptree& pr)
    {
        /* Example received pedestrian tracking data:
        {"dataNumber": "473085", "messageType": "Data", "time": "2022-04-20T15:25:51.001-04:00",
        "track": [{"angle": "263.00000000", "class": "Pedestrian", "iD": "15968646", "latitude": "38.95499217",
        "longitude": "-77.14920953", "speed": "1.41873741", "x": "0.09458912", "y": "14.80903757"}], "type": "PedestrianPresenceTracking"}
        */

        std::string time = pr.get<std::string>("time", "");
        std::string type = pr.get<std::string>("type", "");

        PLOG(logINFO) << "Received " << type << " data at time: " << time;

        if (type == "PedestrianPresenceTracking") {
            processPedestrianData(pr, time);
        }
       

    }

    void FLIRWebSockAsyncClnSession::processPedestrianData(const pt::ptree& pr, const std::string& time)
    {
        try {
            // Declare initial null and J2735 default values if not provided by FLIR.
            int angle = 0;
            float rawAlpha = 0.0f;
            int alpha = 28800; // Default for J2735 unavailable DE_Angle
            std::string lat = "";
            std::string lon = "";
            std::string fixedLat = "";
            std::string fixedLon = "";
            int speed = 8191; // Default for unavailable DE_Speed and DE_Velocity
            int id = 0;
            std::string infraId = "";
            std::string idResult = "";
            std::string objID = "";
            std::string xOffset = ""; // Offset of detected track from FLIR position in m
            std::string yOffset = "";
            std::string detObjXmlStr;
    
            // Parse out seconds from datetime string
            long timestamp = timeStringParser(time); 
    
            for (auto &it: pr.get_child("track")) 
            {
                // Parse angle
                if (!it.second.get_child("angle").data().empty())
                {
                    // Angle only reported in whole number increments, so int is fine
                    angle = std::stoi(it.second.get_child("angle").data());
                    // Convert camera reference frame angle
                    rawAlpha = cameraRotation_ - static_cast<float>(angle) - 270.0f;
                    if (rawAlpha < 0)
                    {
                        rawAlpha = std::fmod(rawAlpha, 360.0f) + 360.0f;
                    }
                    //divide by 0.0125 for J2735 format
                    alpha = static_cast<int>(std::round(rawAlpha / 0.0125f));
                }
    
                // Parse ID
                if (!it.second.get_child("iD").data().empty()) 
                {
                    id = std::stoi(it.second.get_child("iD").data());
                    if (id > 65535)
                    {
                        auto old_id = id;
                        id = id%65535;
                        PLOG(logWARNING) << "ID " << old_id << " out of range. Assigning new ID " << id;
                    }
                    objID = std::to_string(id);
                }
    
                // Parse latitude
                if (!it.second.get_child("latitude").data().empty())
                {
                    // Convert received lat/lon to J2735 lat/lon format
                    lat = it.second.get_child("latitude").data();
                    lat.erase(std::remove(lat.begin(), lat.end(), '.'), lat.end());
                    lat.pop_back();
                }
    
                // Parse longitude
                if (!it.second.get_child("longitude").data().empty())
                {
                    // Convert received lat/lon to J2735 lat/lon format
                    lon = it.second.get_child("longitude").data();
                    lon.erase(std::remove(lon.begin(), lon.end(), '.'), lon.end());
                    lon.pop_back();
                }
                
                if (!it.second.get_child("x").data().empty())
                {
                    // Convert x/y offset in m (FLIR) to 1/10 m (J2735)
                    int xOffsetInt = std::round(std::stod(it.second.get_child("x").data())*10);
                    // Convert received lat/lon to J2735 lat/lon format
                    xOffset = std::to_string(xOffsetInt);
                }
                if (!it.second.get_child("y").data().empty())
                {
                    // Convert x/y offset in m (FLIR) to 1/10 m (J2735)
                    int yOffsetInt = std::round(std::stod(it.second.get_child("y").data())*10);
                    yOffset = std::to_string(yOffsetInt);
                }
    
    
                // Parse speed
                if (!it.second.get_child("speed").data().empty())
                {
                    // Speed from the FLIR camera is reported in m/s
                    // Convert to units of 0.02 m/s for J2735 Speed and Velocity format
                    float rawSpeed = std::stof(it.second.get_child("speed").data());
                    speed = static_cast<int>(std::round(rawSpeed / 0.02f));
                }                  
    
    
    
                PLOG(logINFO) << "Received FLIR camera data for pedestrian " << idResult << " at location: (" << xOffset << ", " << yOffset <<
                              "), traveling at speed: " << speed << ", with heading: " << alpha << " degrees";
    
                std::lock_guard<mutex> lock(_msgLock);
                tmx::messages::SensorDetectedObject obj;
                obj.set_timestamp(timestamp);
                obj.set_objectId(std::stoi(idResult));
                obj.set_type("PEDESTRIAN");
                obj.set_sensorId(hostString_);
                obj.set_projString("+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu");
                //obj.set_timestamp() TODO :Setup method to convert date time to epoch time
                obj.set_wgs84_position( tmx::messages::WGS84Position(std::stod(lat), std::stod(lon), 0.0));
                obj.set_position(tmx::messages::Position(std::stod(xOffset), std::stod(yOffset), 0.0));
                // TODO Convert angle to orientation
                obj.set_orientation(tmx::messages::Orientation(0.0, 0.0, 0.0));
                // TODO Convert angle and speed to velocity
                obj.set_velocity(tmx::messages::Velocity(0.0, 0.0, 0.0));
                // Average pedestrian size standing is 0.5m x 0.6m (https://www.fhwa.dot.gov/publications/research/safety/pedbike/05085/chapt8.cfm)
                obj.set_size(tmx::messages::Size(0.5, 0.6, 0.0));
                // TODO Convert sensor position accuracy to covariance
                std::vector<std::vector< tmx::messages::Covariance>> positionCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
                obj.set_positionCovariance(positionCov);
                // TODO Convert sensor speed and heading accuracy to covariance
                std::vector<std::vector< tmx::messages::Covariance>> velocityCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
                obj.set_velocityCovariance(velocityCov);
                // TODO Convert heading accuracy to covariance
                std::vector<std::vector< tmx::messages::Covariance>> orientationCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
                obj.set_orientationCovariance(orientationCov);
                // Add detection to queue
                msgQueue.push(obj);
            }
        }
        catch(const ptree_error &e) {
            PLOG(logERROR) << "Error with track data:  " << e.what();
        }
    }

    void FLIRWebSockAsyncClnSession::on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");
        isHealthy_.store(false);
        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

    std::queue<tmx::messages::SensorDetectedObject> FLIRWebSockAsyncClnSession::getMsgQueue()
    {
        std::lock_guard<mutex> lock(_msgLock);
        return msgQueue;
    }

    long FLIRWebSockAsyncClnSession::timeStringParser(string dateTimeStr) const
    {
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
  
        int millisecondsTotal = (std::stoi(sec) * 1000) + std::stoi(milliseconds);    
        std::tm t{};
        t.tm_year = std::stoi(year); // Year since 1900
        t.tm_mon = std::stoi(month);           // Month (0-11, so 4 is May)
        t.tm_mday = std::stoi(day);          // Day of the month
        t.tm_hour = std::stoi(hour);          // Hour of the day
        t.tm_min = std::stoi(mins);           // Minute of the hour
        t.tm_sec = std::stoi(sec);

        std::time_t time = std::mktime(&t);
        // Convert to epoch ms
        long epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(time).time_since_epoch()).count() 
            + std::chrono::milliseconds(std::stoi(milliseconds)).count();
        
        return epochMs;
    }       


    bool FLIRWebSockAsyncClnSession::isPedestrainPresent() const{
        return isPedestrainPresent_.load();
    }

    void FLIRWebSockAsyncClnSession::setPedestrainPresence(bool isPresent){
        PLOG(logDEBUG) << "Set pedestrain presence: " << isPresent << ", region/view: " <<cameraViewName_ << std::endl;
        isPedestrainPresent_.store(isPresent);
    }

    bool FLIRWebSockAsyncClnSession::isHealthy() const{
        return isHealthy_.load();
    }
}
