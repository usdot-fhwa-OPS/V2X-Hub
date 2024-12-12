
/**
 * Copyright (C) 2019 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this plogFile except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

 #ifndef TMX_PLUGINS_ODEForwardPlugin_H_
 #define TMX_PLUGINS_ODEForwardPlugin_H_

 #include "PluginClient.h"
 #include "PluginDataMonitor.h"
 #include <iostream>
 #include <cstring>
 #include <string>
 #include <fstream>
 #include <stdio.h>
 #include <stdlib.h>
 #include <chrono>
 #include <atomic>
 #include <thread>
 #include <boost/algorithm/string.hpp>
 #include <tmx/messages/IvpJ2735.h>
 #include <tmx/j2735_messages/BasicSafetyMessage.hpp>
 #include <tmx/j2735_messages/SpatMessage.hpp>
 #include <tmx/j2735_messages/TravelerInformationMessage.hpp>
 #include <tmx/j2735_messages/MapDataMessage.hpp>
 #include <BasicSafetyMessage.h>
 #include <tmx/messages/auto_message.hpp>
 #include <librdkafka/rdkafkacpp.h>
 #include <tmx/json/cJSON.h>
 #include "/usr/local/include/date/date.h"
 #include "UDPMessageForwarder.h"
 #include "CommunicationModeHelper.h"

 
 using namespace std;
 using namespace tmx;
 using namespace tmx::utils;
 using namespace tmx::messages;
 using namespace date;

 namespace ODEForwardPlugin
 {


 #define BYTESTOMB 1048576

 /**
  * This plugin logs the BSM messages received in the following CSV format.
  */
 class ODEForwardPlugin: public PluginClient
 {
 public:
 	ODEForwardPlugin(std::string);
 	virtual ~ODEForwardPlugin();
 	int Main();
 protected:
 	void UpdateConfigSettings();

 	// Virtual method overrides.
 	void OnConfigChanged(const char *key, const char *value);
 	void OnStateChange(IvpPluginState state);

 	void HandleRealTimePublish(BsmMessage &msg, routeable_message &routeableMsg);
 	void HandleSPaTPublish(SpatMessage &msg, routeable_message &routeableMsg);
 	void HandleTimPublish(TimMessage &msg, routeable_message &routeableMsg);
 	void HandleMapPublish(MapDataMessage &msg, routeable_message &routeableMsg);

 private:
 	std::atomic<uint64_t> _frequency{0};
 	DATA_MONITOR(_frequency);   // Declares the

 	void QueueKafkaMessage(RdKafka::Producer *producer, std::string topic, std::string message);
	void sendSpatKafkaMessage(SpatMessage &msg, routeable_message &routeableMsg);
	void sendSpatUDPMessage(SpatMessage &msg, routeable_message &routeableMsg) const;
	void sendBsmKafkaMessage(BsmMessage &msg, routeable_message &routeableMsg);
	void sendBsmUDPMessage(BsmMessage &msg, routeable_message &routeableMsg) const;
	void sendTimUDPMessage(TimMessage &msg, routeable_message &routeableMsg) const;
	void sendMapUDPMessage(MapDataMessage &msg, routeable_message &routeableMsg) const;

 	uint16_t _scheduleFrequency;
	uint16_t _freqCounter;
 	uint16_t _forwardMSG;
 	std::string _BSMkafkaTopic;
 	std::string _SPaTkafkaTopic;
 	std::string _kafkaBrokerIp;
 	std::string _kafkaBrokerPort;
 	std::string kafkaConnectString;
 	RdKafka::Conf *kafka_conf;
 	RdKafka::Producer *kafka_producer;
	int _MAPUDPPort;
	int _TIMUDPPort;
	int _BSMUDPPort;
	int _SPATUDPPort;
	std::string _communicationMode;
	std::string _udpServerIpAddress;
	std::shared_ptr<UDPMessageForwarder> _udpMessageForwarder;
	std::shared_ptr<CommunicationModeHelper> _communicationModeHelper;
 };
 std::mutex _cfgLock;


 } /* namespace ODEForwardPlugin */

 #endif /* ODEForwardPlugin.h */
