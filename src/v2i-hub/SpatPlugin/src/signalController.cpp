/*
 ============================================================================
 Name        : snmpClient.cpp
 Author      : William Gibbs
 Version     :
 Copyright   : Battelle
 Description : Query Signal Controller and populate the SPaT message
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <netinet/in.h>
#ifndef __CYGWIN__
#include <sys/prctl.h>
#endif
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <PluginLog.h>

#include "signalController.h"

#include "NTCIP1202.h"

using namespace tmx::messages;
using namespace tmx::utils;
using namespace std;

SignalController::~SignalController() {	
	SNMPCloseSession();
}

void SignalController::Start(std::string signalGroupMappingJson)
{
	_signalGroupMappingJson = signalGroupMappingJson;

	// Create mutex for the Spat message
	pthread_mutex_init(&spat_message_mutex, NULL);
    // launch update thread
    sigcon_thread_id = boost::thread(&SignalController::start_signalController, this);
    // test code
    normalstate = 0x01;
    crossstate = 0x04;
}

// get sockaddr, IPv4 or IPv6:
void *SignalController::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void SignalController::setConfigs(std::string localIp, std::string localUdpPort, std::string tscIp, std::string tscRemoteSnmpPort, std::string ptlmFile, std::string intersectionName, int intersectionId)
{
	_localIp = strdup(localIp.c_str());
	_localUdpPort = strdup(localUdpPort.c_str());
	_intersectionId = intersectionId;
	_intersectionName = strdup(intersectionName.c_str());
	_tscIp = tscIp;
	_tscRemoteSnmpPort = stoi(tscRemoteSnmpPort);
}

void SignalController::start_signalController()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "SpatGenSC", 0, 0, 0);
#endif

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	int maxDataSize = 1000;

    int sockfd, numbytes;
    char buf[maxDataSize];
    struct addrinfo hints, *servinfo;
    int rv;
    struct timeval tv;
    int on = 1;

		//Enable SPAT
		// 0 = disable
		// 2 = enable SPAT
		// 6 = enable SPAT wit pedestrian data
		PLOG(logINFO) << "Enable SPAT Sent";
		SNMPSet("1.3.6.1.4.1.1206.3.5.2.9.44.1.0", 2);
		SNMPCloseSession();

		// Create UDP Socket
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_DGRAM;
	    hints.ai_protocol = IPPROTO_UDP;
	    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

	    EthernetIsConnected = 0;
	    IsReceiving = 0;

	    while (1) {
		PLOG(logDEBUG) << "Top of While Loop";
			if ((rv = getaddrinfo(_localIp, _localUdpPort, &hints, &servinfo)) != 0) {
			PLOG(logERROR) << "Getaddrinfo Failed " << _localIp << " " << _localUdpPort << ". Exiting thread!!!";
	    		return;
			}
			PLOG(logDEBUG) << "Getting Socket";
			if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,	servinfo->ai_protocol)) == -1) {
			PLOG(logERROR) << "Get Socket Failed " << _localIp << " " << _localUdpPort << ". Exiting thread!!!";
				return;
			}

			rv = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

			// Set the socket to time out on reads if no data comes in during the timeout value then the socket will close and
			// then try to re-open during the normal execution
			// Wait up to 10 seconds.
		    tv.tv_sec = 10;
		    tv.tv_usec = 0;
		    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

			if (bind(sockfd,servinfo->ai_addr,servinfo->ai_addrlen)==-1) {
			PLOG(logERROR) << "Could not bind to Socket " << _localIp << " " << _localUdpPort << ". Exiting thread!!!";
				return;
			}

			if (servinfo == NULL) {
				PLOG(logERROR) << "Could not connect";
				EthernetIsConnected = 0;
			}
			else {
				PLOG(logDEBUG) << "Connected";
				EthernetIsConnected = 1;
			}

			if (EthernetIsConnected) {
				//printf("Signal Controller UDP Client Connected to %s:%s\n",_localIp, _localUdpPort);
				freeaddrinfo(servinfo); // all done with this structure
//				client_socket = sockfd;
				// Receive Packets and process until disconnected
				while(EthernetIsConnected) {
					//printf("Signal Controller ethernet connected, reading data\n");
					numbytes = recv(sockfd, buf, maxDataSize-1, 0);
					//printf("Signal Controller read %d bytes\n", numbytes);
					//TODO - Check the start byte for 0xcd, then check for len of 245.
					//TODO - store in temp space if less than 245, send only from 0xcd (byte 0) to byte 245 to new processing function
					if ((numbytes == -1) || (numbytes == 0)){
						if(numbytes == 0)
							PLOG(logINFO) << "Signal Controller Timed out";
						else
							PLOG(logINFO) << "Signal Controller Client closed";
						EthernetIsConnected = 0;
						IsReceiving = 0;
					}
					else {

						IsReceiving = 1;
						pthread_mutex_lock(&spat_message_mutex);
						auto ntcip1202 = std::make_shared<Ntcip1202>();
						ntcip1202->setSignalGroupMappingList(_signalGroupMappingJson);
						//printf("Signal Controller calling ntcip1202 copyBytesIntoNtcip1202");
						ntcip1202->copyBytesIntoNtcip1202(buf, numbytes);

						//printf("Signal Controller calling ntcip1202 ToJ2735r41SPAT");
						SPAT *_spat = (SPAT *) calloc(1, sizeof(SPAT));
						ntcip1202->ToJ2735r41SPAT(_spat, _intersectionName, _intersectionId);
						
						//printf("Signal Controller calling _spatMessage set_j2735_data\n");
						//_spatMessage.set_j2735_data(_spat);
						if (_spatMessage != NULL)
						{
							// delete _spatMessage;
							_spatMessage = NULL;
						}
						_spatMessage = std::make_shared<tmx::messages::SpatMessage>(_spat);

						pthread_mutex_unlock(&spat_message_mutex);
						PLOG(logDEBUG) << *_spatMessage;
					}
				}
			}
			sleep(3);
	    }
}

void SignalController::getEncodedSpat(SpatEncodedMessage* spatEncodedMsg, std::string currentPedLanes)
{
	pthread_mutex_lock(&spat_message_mutex);

	//printf("Signal Controller getEncodedSpat\n");
	if (_spatMessage != NULL) {
		// Add pedestrian lanes with active detections and clear the rest
		auto spat = _spatMessage->get_j2735_data();
		if (spat && spat->intersections.list.array && spat->intersections.list.count > 0) {
			char *zoneList = strdup(currentPedLanes.c_str());
			vector<LaneConnectionID_t> zones;
			char *c = strtok(zoneList, ",");

			while (c != NULL) {
				zones.push_back(strtol(c, NULL, 0));

				c = strtok(NULL, ",");
			};

			free(zoneList);
			zoneList = NULL;
			c = NULL;

			if (!zones.empty()) {
				ManeuverAssistList *&mas = spat->intersections.list.array[0]->maneuverAssistList;
				mas = (ManeuverAssistList *) calloc(1, sizeof(ManeuverAssistList));

				mas->list.count = zones.size();
				mas->list.array = (ConnectionManeuverAssist **) calloc(1, sizeof(ConnectionManeuverAssist *));

				std::sort(zones.begin(), zones.end());
				for (size_t i = 0; i < zones.size(); i++) {
					mas->list.array[i] = (ConnectionManeuverAssist *) calloc(1, sizeof(ConnectionManeuverAssist));
					mas->list.array[i]->connectionID = zones[i];
					mas->list.array[i]->pedBicycleDetect = (PedestrianBicycleDetect_t *) calloc(1, sizeof(PedestrianBicycleDetect_t));
					*(mas->list.array[i]->pedBicycleDetect) = 1;
				}
			}
		}

		MessageFrameMessage frame(_spatMessage->get_j2735_data());
		spatEncodedMsg->set_data(TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame));
		//Free the memory allocated for MessageFrame
		free(frame.get_j2735_data().get());
	}

	pthread_mutex_unlock(&spat_message_mutex);

}

int SignalController::getIsConnected()
{
	return EthernetIsConnected && IsReceiving;
}

int SignalController::getActionNumber()
{
	return 1;//sd.actionNumber;
}

void SignalController::SNMPOpenSession()
{
	//check for valid TSC info
	if (_tscIp == "" || _tscRemoteSnmpPort == 0)
		return;
	//open snmp session
	snmp_sess_init(&_session_info);
	string peername = _tscIp;
	peername.append(":");
	peername.append(to_string(_tscRemoteSnmpPort));
	_session_info.peername = (char*)peername.c_str();
	_session_info.version = SNMP_VERSION_1;
	_session_info.community = (u_char*)"public";
	_session_info.community_len = strlen("public");
	_session = snmp_open(&_session_info);
	if (_session)
		_snmpSessionOpen = true;
}

void SignalController::SNMPCloseSession()
{
	//close session
	if (_snmpSessionOpen)
		snmp_close(_session);
	_snmpSessionOpen = false;
}

bool SignalController::SNMPSet(string targetOid, int32_t value)
{
	return SNMPSet(targetOid, ASN_INTEGER, (const void *)&value, sizeof(value));
}

bool SignalController::SNMPSet(string targetOid, u_char type, const void *value, size_t len)
{
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;
	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;
	int status;
	bool rc = true;

	//check is snmp session open
	if (!_snmpSessionOpen)
	{
		SNMPOpenSession();
		if (!_snmpSessionOpen)
			return false;
		_snmpDestinationChanged = false;
	}
	//check destination change
	if (_snmpDestinationChanged)
	{
		SNMPCloseSession();
		SNMPOpenSession();
		if (!_snmpSessionOpen)
			return false;
		_snmpDestinationChanged = false;
	}

	pdu = snmp_pdu_create(SNMP_MSG_SET);
	read_objid(targetOid.c_str(), anOID, &anOID_len);
	snmp_pdu_add_variable(pdu, anOID, anOID_len, type, value, len);
	status = snmp_synch_response(_session, pdu, &response);
	if (status != STAT_SUCCESS || response->errstat != SNMP_ERR_NOERROR)
		rc = false;
	if (response)
		snmp_free_pdu(response);

	return rc;
}

