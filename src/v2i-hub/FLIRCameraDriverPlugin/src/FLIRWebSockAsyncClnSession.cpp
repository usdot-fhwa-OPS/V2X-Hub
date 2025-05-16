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
            double angle = 0;
            double ned_heading = 0.0;
            double convertedCameraRotation =  360 - (cameraRotation_);

            double lat = 0.0;
            double lon = 0.0;
            double speed = 0.0;
            double velocityX = 0.0;
            double velocityY = 0.0;
            double offsetX = 0.0;
            double offsetY = 0.0;
            double correctOffsetX = 0.0;
            double correctOffsetY = 0.0;
            int id = 0;
    
            // Parse out seconds from datetime string
            long timestamp = timeStringParser(time); 
    
            for (auto &it: pr.get_child("track")) 
            {
                // Parse angle
                if (!it.second.get_child("angle").data().empty())
                {
                    // Angle is in degrees in camera coordinates
                    angle = std::stod(it.second.get_child("angle").data());
                    // Convert camera reference frame angle
                    // Assume camera rotation is NED (negative from true north)
                    // Convert to ENU (positive from true east)
                    // +90 for considering angle from east, subtract 90 for FLIR camera axis rotation.
                    // Subtract camera rotation from 360 since FLIR camera rotation is in NED and ENU is
                    // opposite direction
                    ned_heading = angle + convertedCameraRotation;
                    if (ned_heading < 0 )
                    {
                        ned_heading = std::fmod(ned_heading, 360.0f) + 360.0f;
                    }
                    else if (ned_heading > 360)
                    {
                        ned_heading = std::fmod(ned_heading, 360.0f);
                    }
                    
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
                }
    
                // Parse latitude
                if (!it.second.get_child("latitude").data().empty())
                {
                    // Latitude is in degrees
                    lat = std::stod(it.second.get_child("latitude").data());
                }
    
                // Parse longitude
                if (!it.second.get_child("longitude").data().empty())
                {
                    // Longitude is in degrees
                    lat = std::stod(it.second.get_child("longitude").data());
                }
                
                if (!it.second.get_child("x").data().empty())
                {
                    // Offset in meters camera coordinates
                    offsetX = std::stod(it.second.get_child("x").data());
                    
                    // Calculate ENU x offset using converted camera rotation
                    correctOffsetX = offsetX * std::cos( convertedCameraRotation* M_PI / 180.0) -
                        offsetY * std::sin(convertedCameraRotation * M_PI / 180.0);
                }
                if (!it.second.get_child("y").data().empty())
                {
                    // Offset in meters camera coordinates
                    int offsetY = std::stod(it.second.get_child("y").data());
                    // Calculate ENU y offset using converted camera rotation
                    correctOffsetY = offsetX * std::sin( convertedCameraRotation* M_PI / 180.0) +
                        offsetY * std::cos(convertedCameraRotation * M_PI / 180.0);
                }

    
                // Parse speed
                if (!it.second.get_child("speed").data().empty())
                {
                    // Speed is in m/s
                    speed = std::stod(it.second.get_child("speed").data());
                    // Get velocity from speed and angle
                    velocityX = speed * std::cos(ned_heading * M_PI / 180.0);
                    velocityY = speed * std::sin(ned_heading * M_PI / 180.0);
                }                  
    
                tmx::messages::SensorDetectedObject obj;
                obj.set_timestamp(timestamp);
                obj.set_objectId(id);
                obj.set_type("PEDESTRIAN");
                obj.set_sensorId(cameraViewName_);
                obj.set_projString("+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu");
                obj.set_wgs84_position( tmx::messages::WGS84Position(lat, lon, 0.0));
                obj.set_position(tmx::messages::Position(correctOffsetX, correctOffsetY, 0.0));
                // Convert angle to orientation
                obj.set_orientation(tmx::messages::Orientation(std::cos(ned_heading * M_PI / 180.0), std::sin(ned_heading * M_PI / 180.0), 0.0));
                // Convert angle and speed to velocity
                obj.set_velocity(tmx::messages::Velocity(velocityX, velocityY, 0.0));
                // Average pedestrian size standing is 0.5m x 0.6m (https://www.fhwa.dot.gov/publications/research/safety/pedbike/05085/chapt8.cfm)
                obj.set_size(tmx::messages::Size(0.5, 0.6, 0.0));
                // TODO Convert sensor position accuracy to covariance
                std::vector<std::vector< tmx::messages::Covariance>> positionCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
                positionCov[0][0] = tmx::messages::Covariance(0.5); // x
                positionCov[1][1] = tmx::messages::Covariance(0.5); // y
                positionCov[2][2] = tmx::messages::Covariance(0.5); // z
                obj.set_positionCovariance(positionCov);
                // TODO Convert sensor speed and heading accuracy to covariance
                std::vector<std::vector< tmx::messages::Covariance>> velocityCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
                velocityCov[0][0] = tmx::messages::Covariance(1); // x
                velocityCov[1][1] = tmx::messages::Covariance(1); // y
                velocityCov[2][2] = tmx::messages::Covariance(1); // z
                obj.set_velocityCovariance(velocityCov);
                // TODO Convert heading accuracy to covariance
                std::vector<std::vector< tmx::messages::Covariance>> orientationCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
                orientationCov[0][0] = tmx::messages::Covariance(5); // x
                orientationCov[1][1] = tmx::messages::Covariance(5); // y
                orientationCov[2][2] = tmx::messages::Covariance(5); // z
                obj.set_orientationCovariance(orientationCov);
                // Checkout lock to modify queue
                std::lock_guard<mutex> lock(_msgLock);
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

    void FLIRWebSockAsyncClnSession::clearMsgQueue()
    {
        std::lock_guard<mutex> lock(_msgLock);
        msgQueue = std::queue<tmx::messages::SensorDetectedObject>();
    }

    uint64_t FLIRWebSockAsyncClnSession::timeStringParser(string dateTimeStr) const
    {
        std::regex re(R"((\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})\.(\d+)([+-])(\d{2}):(\d{2}))");
        std::smatch match;

        if (!std::regex_match(dateTimeStr, match, re)) {
            throw std::invalid_argument("Invalid datetime format");
        }

        std::tm t = {};
        t.tm_year = std::stoi(match[1]) - 1900;
        t.tm_mon  = std::stoi(match[2]) - 1;
        t.tm_mday = std::stoi(match[3]);
        t.tm_hour = std::stoi(match[4]);
        t.tm_min  = std::stoi(match[5]);
        t.tm_sec  = std::stoi(match[6]);

        int milliseconds = std::stoi(match[7]);
        std::string offset_sign = match[8];
        int offset_hours = std::stoi(match[9]);
        int offset_minutes = std::stoi(match[10]);

        std::time_t time = std::mktime(&t);
        // Convert to epoch ms
        uint64_t epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(time).time_since_epoch()).count() 
            + std::chrono::milliseconds(milliseconds).count();
        
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
