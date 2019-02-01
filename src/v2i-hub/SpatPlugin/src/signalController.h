/*
 * snmpClient.h
 *
 *  Created on: Aug 22, 2014
 *      Author: gibbsw
 */

#ifndef SIGNALCONTROLLER_H_
#define SIGNALCONTROLLER_H_

#include <pthread.h>
#include <boost/thread.hpp>

#include <tmx/j2735_messages/SpatMessage.hpp>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>


class SignalController
{
	public:
		~SignalController();

		void Start(std::string signalGroupMappingJson);
		void spat_load();
		void start_signalController();
		int getActionNumber();
		void setConfigs(std::string ip, std::string udpPort, std::string snmpIP, std::string snmpPort, std::string ptlmFile, std::string intersectionName, int intersectionId);
		void updatePtlmFile(const char* ptlmFile);
		int getIsConnected();

		//int getDerEncodedSpat(unsigned char* derEncodedBuffer);

		void getEncodedSpat(tmx::messages::SpatEncodedMessage* spatEncodedMsg, std::string currentPedLanes = "");

		pthread_mutex_t spat_message_mutex;
		boost::thread sigcon_thread_id;

		void SNMPOpenSession();
		void SNMPCloseSession();
		bool SNMPSet(std::string targetOid, int32_t value);
		bool SNMPSet(std::string targetOid, u_char type, const void *value, size_t len);

	private:
		void *get_in_addr(struct sockaddr *);

		// Local IP address and UDP port for reception of SPAT dSPaTDataata from the TSC.
		char* _localIp;
		char* _localUdpPort;
		char* _intersectionName;
		int _intersectionId;
		std::string _tscIp;
		uint32_t _tscRemoteSnmpPort;

		std::string _signalGroupMappingJson;
		tmx::messages::SpatMessage *_spatMessage{NULL};

		int counter;
		unsigned long normalstate;
		unsigned long crossstate;
		int EthernetIsConnected;
		int IsReceiving;

		//snmp
		struct snmp_session _session_info;
		struct snmp_session *_session{NULL};
		bool _snmpSessionOpen{false};
		bool _snmpDestinationChanged{false};

};

#endif /* SIGNALCONTROLLER_H_ */
