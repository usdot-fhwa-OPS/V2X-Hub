#include "include/FLIRWebSockAsyncClnSession.hpp"

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace PedestrianPlugin
{
    int 
    FLIRWebSockAsyncClnSession::calculateMinuteOfYear(int year, int month, int day, int hour, int minute, int second) {
    // Set up current time
    std::tm currentTime = {};
    currentTime.tm_year = year - 1900;
    currentTime.tm_mon = month - 1;
    currentTime.tm_mday = day;
    currentTime.tm_hour = hour;
    currentTime.tm_min = minute;
    currentTime.tm_sec = second;

    // Set up start of the year
    std::tm startOfYear = {};
    startOfYear.tm_year = year - 1900;
    startOfYear.tm_mon = 0;
    startOfYear.tm_mday = 1;
    startOfYear.tm_hour = 0;
    startOfYear.tm_min = 0;
    startOfYear.tm_sec = 0;

    // Calculate difference in seconds
    std::time_t current = std::mktime(&currentTime);
    std::time_t start = std::mktime(&startOfYear);
    if (current == -1 || start == -1) {
        throw std::runtime_error("Failed to compute time difference");
    }

    // Convert seconds to minutes
    int secondsDifference = static_cast<int>(std::difftime(current, start));
    int minuteOfYear = secondsDifference / 60;

    return minuteOfYear;
    }

    void
    FLIRWebSockAsyncClnSession::fail(beast::error_code ec, char const* what) const
    {
        PLOG(logDEBUG) << what << ": " << ec.message();
    }

    void
    FLIRWebSockAsyncClnSession::run(
        char const* host,
        char const* port,
        float cameraRotation,
        char const* hostString)
    {
        
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::run ";
	    // Save these for later
        host_ = host;
        cameraRotation_ = cameraRotation;
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

    void
    FLIRWebSockAsyncClnSession::on_resolve(
         beast::error_code ec,
         tcp::resolver::results_type results)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_resolve ";
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
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_connect ";
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

    void
    FLIRWebSockAsyncClnSession::on_handshake(beast::error_code ec)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_handshake ";
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
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_write ";
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
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_read ";
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        // parse the buffer
        std::stringstream ss; 
	    ptree pr; 
	    std::string text(beast::buffers_to_string(buffer_.data()));
        ss<<text; 

        PLOG(logDEBUG) << "Received:  " << text.c_str();

        try
        {
	        pt::read_json(ss, pr);
        }
        catch(const ptree_error &e)
        {
            PLOG(logERROR) << "Error converting json to p tree:  " << e.what();
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
        std::string idResult = "";
        int mappedID = 0;

        if (messageType.compare("Subscription") == 0)
        {
            std::string subscrStatus = pr.get_child("subscription").get_child("returnValue").get_value<string>();
            PLOG(logDEBUG) << "Ped presence data subscription status: " << subscrStatus;
        
        }
        //Example received pedestrian tracking data       
        //{"dataNumber": "473085", "messageType": "Data", "time": "2022-04-20T15:25:51.001-04:00", 
        //"track": [{"angle": "263.00000000", "class": "Pedestrian", "iD": "15968646", "latitude": "38.95499217", 
        //"longitude": "-77.14920953", "speed": "1.41873741", "x": "0.09458912", "y": "14.80903757"}], "type": "PedestrianPresenceTracking"}
        else if (messageType.compare("Data") == 0)
        {
            time = pr.get_child("time").get_value<string>(); 
            type =  pr.get_child("type").get_value<string>();

            PLOG(logINFO) << "Received " << type << " data at time: " << time;

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

                            // Need to convert ID for SDSM
                            // Map the value to be less than 65535
                            int mappedID = static_cast<int>(id % 65535);

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
                        << " " << dateTimeArr[3] << ":" << dateTimeArr[4] << ":" << dateTimeArr[5] << ":" << dateTimeArr[6];  

                        PLOG(logINFO) << "Received FLIR camera data for pedestrian " << idResult << " at location: (" << lat << ", " << lon <<
                        ")" << ", travelling at speed: " << speed << ", with heading: " << alpha << " degrees";

                        PLOG(logINFO) << "Message count: " << msgCount;

                        /* NOTE: Removed PSM constuction from FLIR input. Keeping as reference and if returned use is desired.
                        // Constructing PSM XML to send to BroadcastPedDet function
                        char psm_xml_char[10000]; 
                        snprintf(psm_xml_char,10000,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><PersonalSafetyMessage><basicType><aPEDESTRIAN/></basicType><secMark>%i</secMark><msgCnt>%i</msgCnt><id>%s</id><position><lat>%s</lat><long>%s</long></position><accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy><speed>%.0f</speed><heading>%i</heading><pathHistory><initialPosition><utcTime><year>%i</year><month>%i</month><day>%i</day><hour>%i</hour><minute>%i</minute><second>%i</second></utcTime><long>0</long><lat>0</lat></initialPosition><crumbData><PathHistoryPoint><latOffset>0</latOffset><lonOffset>0</lonOffset><elevationOffset>0</elevationOffset><timeOffset>1</timeOffset></PathHistoryPoint></crumbData></pathHistory></PersonalSafetyMessage>", dateTimeArr[6], msgCount, idResult.c_str(), lat.c_str(), lon.c_str(), speed, alpha, dateTimeArr[0], dateTimeArr[1], dateTimeArr[2], dateTimeArr[3], dateTimeArr[4], dateTimeArr[6]);
                        std::string psm_xml_str(psm_xml_char, sizeof(psm_xml_char) / sizeof(psm_xml_char[0]));
                        */

                        // Constructing SDSM XML to send to BroadcastPedDet function
                        char sdsm_xml_char[10000];
                        snprintf(sdsm_xml_char,10000,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><SensorDataSharingMessage><msgCnt>%i</msgCnt><sourceID>%s</sourceID><equipmentType><rsu/></equipmentType><sDSMTimeStamp><year>%i</year><month>%i</month><day>%i</day><hour>%i</hour><minute>%i</minute><second>%i</second></sDSMTimeStamp><refPos><lat>%s</lat><long>%s</long></refPos><refPosXYConf><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></refPosXYConf><objects><DetectedObjectData><detObjCommon><objType><vru/></objType><objTypeCfd>98</objTypeCfd><objectID>%i</objectID><measurementTime>0</measurementTime><timeConfidence><time-000-001/></timeConfidence><pos><offsetX>0</offsetX><offsetY>0</offsetY></pos><posConfidence><pos><a20cm/></pos><elevation><elev-000-20/></elevation></posConfidence><speed>1</speed><speedConfidence><prec0-1ms/></speedConfidence><heading>%i</heading><headingConf><prec05deg/></headingConf></detObjCommon></DetectedObjectData></objects></SensorDataSharingMessage>", msgCount, idResult.c_str(), dateTimeArr[0], dateTimeArr[1], dateTimeArr[2], dateTimeArr[3], dateTimeArr[4], dateTimeArr[6], lat.c_str(), lon.c_str(), mappedID, alpha);

                        std::string sdsm_xml_str(sdsm_xml_char, sizeof(sdsm_xml_char) / sizeof(sdsm_xml_char[0]));

                        // Constructing TIM XML to send to BroadcastPedDet function
                        try {
                            moy = calculateMinuteOfYear(dateTimeArr[0], dateTimeArr[1], dateTimeArr[2], dateTimeArr[3], dateTimeArr[4], dateTimeArr[6]);
                            PLOG(logDEBUG) << "Minute of the year: " << moy;
                            } 
                        catch (const std::exception& e) {
                            PLOG(logERROR) << "Error: " << e.what();
                        }
                        char tim_xml_char[10000];
                        snprintf(tim_xml_char,10000,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><TravelerInformation><msgCnt>%i</msgCnt><packetID>0000000000%s</packetID><dataFrames><TravelerDataFrame><notUsed>0</notUsed><frameType><advisory/></frameType><msgId><roadSignID><position><lat>%s</lat><long>%s</long><elevation>-4096</elevation></position><viewAngle>1111111111111111</viewAngle><mutcdCode><warning/></mutcdCode></roadSignID></msgId><startYear>%i</startYear><startTime>%i</startTime><durationTime>1</durationTime><priority>7</priority><notUsed1>0</notUsed1><regions><GeographicalPath><anchor><lat>%s</lat><long>%s</long><elevation>-4096</elevation></anchor><directionality><both/></directionality><description><geometry><direction>1111111111111111</direction><laneWidth>366</laneWidth><circle><center><lat>%s</lat><long>%s</long><elevation>-4096</elevation></center><radius>3000</radius><units><centimeter/></units></circle></geometry></description></GeographicalPath></regions><notUsed2>0</notUsed2><notUsed3>0</notUsed3><content><advisory><SEQUENCE><item><itis>9486</itis></item></SEQUENCE><SEQUENCE><item><itis>13585</itis></item></SEQUENCE></advisory></content></TravelerDataFrame></dataFrames></TravelerInformation>", msgCount, idResult.c_str(), lat.c_str(), lon.c_str(), dateTimeArr[0], moy, lat.c_str(), lon.c_str(), lat.c_str(), lon.c_str());

                        std::string tim_xml_str(tim_xml_char, sizeof(tim_xml_char) / sizeof(tim_xml_char[0]));

		                std::lock_guard<mutex> lock(_msgLock);
                        /* NOTE: Removed PSM constuction from FLIR input. Keeping as reference and if returned use is desired.
                        psmxml = psm_xml_str;
                        msgQueue.push(psmxml);
                        */
                        sdsmxml = sdsm_xml_str;
                        timxml = tim_xml_str;
                        msgQueue.push(sdsmxml);
                        msgQueue.push(timxml);

                        PLOG(logDEBUG) << "Sent XMLs to BroadcastPedDet: " << sdsmxml.c_str() << std::endl << timxml.c_str();
                    }
                }
                catch(const ptree_error &e)
                {
                    PLOG(logERROR) << "Error with track data:  " << e.what();
                }
            }            
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

    void
    FLIRWebSockAsyncClnSession::on_close(beast::error_code ec)
    {
        PLOG(logDEBUG) << "In FLIRWebSockAsyncClnSession::on_close ";
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
