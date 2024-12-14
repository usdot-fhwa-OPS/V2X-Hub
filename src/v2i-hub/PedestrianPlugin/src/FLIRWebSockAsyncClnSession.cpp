#include "include/FLIRWebSockAsyncClnSession.hpp"

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace PedestrianPlugin
{
    const int FLIRWebSockAsyncClnSession::calculateMinuteOfYear(int year, int month, int day, int hour, int minute, int second)
    {
        // Set up "current" utc time using values received from FLIR
        std::tm currentTime = {};
        currentTime.tm_year = year - 1900; // Start of all time in this library is 1900. This calculation is specified in the tm library.
        currentTime.tm_mon = month - 1;
        currentTime.tm_mday = day;
        currentTime.tm_hour = hour;
        currentTime.tm_min = minute;
        currentTime.tm_sec = second;

        // Set up start of the "current" year in utc using the year received from FLIR. 
        // Other values set to the beginning of any year, e.g. <Year>-Jan-01 00:00:00.
        std::tm startOfYear = {};
        startOfYear.tm_year = year - 1900;
        startOfYear.tm_mon = 0;
        startOfYear.tm_mday = 1;
        startOfYear.tm_hour = 0;
        startOfYear.tm_min = 0;
        startOfYear.tm_sec = 0;

        // Calculate difference in seconds. Used to get the current second of the current year.
        std::time_t current = std::mktime(&currentTime);
        std::time_t start = std::mktime(&startOfYear);
        if (current == -1 || start == -1) 
        {
            throw std::invalid_argument("Error with formatting time. Incorrect values provided.");
        }

        // Convert seconds to minutes. Used to get the current minute of the current year.
        auto secondsDifference = static_cast<int>(std::difftime(current, start));
        int minuteOfYear = secondsDifference / 60;

        return minuteOfYear;
    }

    void FLIRWebSockAsyncClnSession::fail(beast::error_code ec, char const* what) const
    {
        PLOG(logERROR) << what << ": " << ec.message();
    }

    void FLIRWebSockAsyncClnSession::run(char const* host, char const* port, float cameraRotation, char const* hostString, bool generatePSM, bool generateSDSM, bool generateTIM)
    {
	    // Save these for later
        host_ = host;
        cameraRotation_ = cameraRotation;
        hostString_ = hostString;
        generatePSM_ = generatePSM;
        generateSDSM_ = generateSDSM;
        generateTIM_ = generateTIM;

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

    void FLIRWebSockAsyncClnSession::handleSubscriptionMessage(const pt::ptree& pr)
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
        for (auto &it: pr.get_child("track")) 
        {
            // Declare initial null and J2735 default values if not provided by FLIR.
            float angle = 0;
            int alpha = 28800; // Default for J2735 unavailable DE_Angle
            std::string lat = "";
            std::string lon = "";
            int speed = 8191; // Default for unavailable DE_Speed and DE_Velocity
            int id = 0;
            std::string idResult = "";
            std::string objID = "";

            // Parse angle
            if (!it.second.get_child("angle").data().empty()) 
            {
                // Angle only reported in whole number increments, so int is fine
                angle = std::stof(it.second.get_child("angle").data());
                // Convert camera reference frame angle
                alpha = cameraRotation_ - static_cast<int>(angle) - 270;
                if (alpha < 0) 
                {
                    alpha = (alpha % 360) + 360;
                }

                //divide by 0.0125 for J2735 format
                alpha /= 0.0125;
            }

            // Parse ID
            if (!it.second.get_child("iD").data().empty()) 
            {
                id = std::stoi(it.second.get_child("iD").data());

                // Convert the id to 4 octet string
                std::stringstream idstream;
                idstream << std::hex << id;
                idResult = idstream.str();
                int str_length_diff = 8 - static_cast<int>(idResult.length());
                idResult.append(str_length_diff, '0');

                // Need to convert ID for SDSM
                // Map the value to be less than 65535
                auto mappedID = static_cast<int>(id % 65535);
                objID = std::to_string(mappedID);
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

            // Parse speed
            if (!it.second.get_child("speed").data().empty())
            {
                // Speed from the FLIR camera is reported in m/s
                // Convert to units of 0.02 m/s for J2735 Speed and Velocity format
                float rawSpeed = std::stof(it.second.get_child("speed").data());
                speed = static_cast<int>(std::round(rawSpeed / 0.02f));
            }

            // Parse out seconds from datetime string
            std::vector<int> dateTimeArr = timeStringParser(time);                       

            msgCount += 1;
            if (msgCount > 127) {
                msgCount = 0;
            }

            PLOG(logINFO) << "Received FLIR camera data at: " << dateTimeArr[0] << "/" << dateTimeArr[1] << "/" << dateTimeArr[2]
                          << " " << dateTimeArr[3] << ":" << dateTimeArr[4] << ":" << dateTimeArr[5] << ":" << dateTimeArr[6];  

            PLOG(logINFO) << "Received FLIR camera data for pedestrian " << idResult << " at location: (" << lat << ", " << lon <<
                          "), traveling at speed: " << speed << ", with heading: " << alpha << " degrees";

            PLOG(logINFO) << "Message count: " << msgCount;
            PLOG(logDEBUG) << "Sent XMLs to BroadcastPedDet: ";

            std::lock_guard<mutex> lock(_msgLock);
            if (generatePSM_ == true)
            {
                // Constructing PSM XML to send to BroadcastPedDet function
                std::string psm_xml_str = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><PersonalSafetyMessage><basicType><aPEDESTRIAN/></basicType><secMark>" + std::to_string(dateTimeArr[6]) + "</secMark><msgCnt>" + std::to_string(msgCount) + "</msgCnt><id>" + idResult + "</id><position><lat>" + lat + "</lat><long>" + lon + "</long></position><accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy><speed>" + std::to_string(speed) + "</speed><heading>" + std::to_string(alpha) + "</heading><pathHistory><initialPosition><utcTime><year>" + std::to_string(dateTimeArr[0]) + "</year><month>" + std::to_string(dateTimeArr[1]) + "</month><day>" + std::to_string(dateTimeArr[2]) + "</day><hour>" + std::to_string(dateTimeArr[3]) + "</hour><minute>" + std::to_string(dateTimeArr[4]) + "</minute><second>" + std::to_string(dateTimeArr[6]) + "</second></utcTime><long>0</long><lat>0</lat></initialPosition><crumbData><PathHistoryPoint><latOffset>0</latOffset><lonOffset>0</lonOffset><elevationOffset>0</elevationOffset><timeOffset>1</timeOffset></PathHistoryPoint></crumbData></pathHistory></PersonalSafetyMessage>";
                PLOG(logDEBUG) << std::endl << psm_xml_str;
                psmxml = psm_xml_str;
                msgQueue.push(psmxml);
            }
            if (generateSDSM_ == true)
            {
                // Constructing SDSM XML to send to BroadcastPedDet function
                std:: string sdsm_xml_str = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><SensorDataSharingMessage><msgCnt>" + std::to_string(msgCount) + "</msgCnt><sourceID>" + idResult + "</sourceID><equipmentType><rsu/></equipmentType><sDSMTimeStamp><year>" + std::to_string(dateTimeArr[0]) + "</year><month>" + std::to_string(dateTimeArr[1]) + "</month><day>" + std::to_string(dateTimeArr[2]) + "</day><hour>" + std::to_string(dateTimeArr[3]) + "</hour><minute>" + std::to_string(dateTimeArr[4]) + "</minute><second>" + std::to_string(dateTimeArr[6]) + "</second></sDSMTimeStamp><refPos><lat>" + lat + "</lat><long>" + lon + "</long></refPos><refPosXYConf><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></refPosXYConf><objects><DetectedObjectData><detObjCommon><objType><vru/></objType><objTypeCfd>98</objTypeCfd><objectID>" + objID + "</objectID><measurementTime>0</measurementTime><timeConfidence><time-000-001/></timeConfidence><pos><offsetX>0</offsetX><offsetY>0</offsetY></pos><posConfidence><pos><a20cm/></pos><elevation><elev-000-20/></elevation></posConfidence><speed>" + std::to_string(speed) + "</speed><speedConfidence><prec0-1ms/></speedConfidence><heading>" + std::to_string(alpha) + "</heading><headingConf><prec05deg/></headingConf></detObjCommon></DetectedObjectData></objects></SensorDataSharingMessage>";
                PLOG(logDEBUG) << std::endl << sdsm_xml_str;
                sdsmxml = sdsm_xml_str;
                msgQueue.push(sdsmxml);
            }
            if (generateTIM_ == true)
            {
                // Constructing TIM XML to send to BroadcastPedDet function
                try
                {
                    moy = calculateMinuteOfYear(dateTimeArr[0], dateTimeArr[1], dateTimeArr[2], dateTimeArr[3], dateTimeArr[4], dateTimeArr[6]);
                    PLOG(logDEBUG) << "Minute of the year: " << moy;
                }
                catch (const std::exception& e)
                {
                    PLOG(logERROR) << "Error: " << e.what();
                }
                std::string tim_xml_str = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><TravelerInformation><msgCnt>" + std::to_string(msgCount) + "</msgCnt><packetID>0000000000" + idResult + "</packetID><dataFrames><TravelerDataFrame><notUsed>0</notUsed><frameType><advisory/></frameType><msgId><roadSignID><position><lat>" + lat + "</lat><long>" + lon + "</long><elevation>-4096</elevation></position><viewAngle>1111111111111111</viewAngle><mutcdCode><warning/></mutcdCode></roadSignID></msgId><startYear>" + std::to_string(dateTimeArr[0]) + "</startYear><startTime>" + std::to_string(moy) + "</startTime><durationTime>1</durationTime><priority>7</priority><notUsed1>0</notUsed1><regions><GeographicalPath><anchor><lat>" + lat + "</lat><long>" + lon + "</long><elevation>-4096</elevation></anchor><directionality><both/></directionality><description><geometry><direction>1111111111111111</direction><laneWidth>366</laneWidth><circle><center><lat>" + lat + "</lat><long>" + lon + "</long><elevation>-4096</elevation></center><radius>3000</radius><units><centimeter/></units></circle></geometry></description></GeographicalPath></regions><notUsed2>0</notUsed2><notUsed3>0</notUsed3><content><advisory><SEQUENCE><item><itis>9486</itis></item></SEQUENCE><SEQUENCE><item><itis>13585</itis></item></SEQUENCE></advisory></content></TravelerDataFrame></dataFrames></TravelerInformation>";
                PLOG(logDEBUG) << std::endl << tim_xml_str;
                timxml = tim_xml_str;
                msgQueue.push(timxml);
            }
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

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

    std::queue<std::string> FLIRWebSockAsyncClnSession::getMsgQueue()
    {
        std::lock_guard<mutex> lock(_msgLock);

        //pass copy of the queue to Pedestrian Plugin
        std::queue<std::string> queueToPass = msgQueue;

        //empty the queue internally
        std::queue<std::string> empty;
        std::swap(msgQueue, empty);
        
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
  
        int millisecondsTotal = (std::stoi(sec) * 1000) + std::stoi(milliseconds);

        parsedArr.push_back(std::stoi(year));
        parsedArr.push_back(std::stoi(month));
        parsedArr.push_back(std::stoi(day));
        parsedArr.push_back(std::stoi(hour));
        parsedArr.push_back(std::stoi(mins));
        parsedArr.push_back(std::stoi(sec));
        parsedArr.push_back(millisecondsTotal);      

        return parsedArr;
    }       
}
