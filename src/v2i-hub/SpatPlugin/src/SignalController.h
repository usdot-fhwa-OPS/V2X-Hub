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
#include <tmx/TmxUtils/src/SNMPClient.h>
#include <tmx/TmxUtils/src/UdpServer.h>
#include "carma-clock/carma_clock.h"

class SignalController
{
	public:
		inline explicit SignalController(std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock) : clock(clock) {};
		~SignalController();

		void Start(std::string signalGroupMappingJson);
		void start_signalController();
		void pollSignalControllerConfiguration();
		void setConfigs(std::string ip, std::string udpPort, std::string snmpIP, std::string snmpPort, std::string intersectionName, int intersectionId);
		int getIsConnected();



	private:

		std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock;
		std::unique_ptr<tmx::utils::snmp_client> snmpClent;
		std::unique_ptr<tmx::utils::UdpServer> udpServer;
		// Local IP address and UDP port for reception of SPAT from the TSC.
		std::string _localIp;
		uint _localUdpPort;
		std::string _intersectionName;
		uint _intersectionId;
		std::string _tscIp;
		uint32_t _tscRemoteSnmpPort;

		std::string _signalGroupMappingJson;
		int EthernetIsConnected;
		int IsReceiving;

		

};

#endif /* SIGNALCONTROLLER_H_ */
