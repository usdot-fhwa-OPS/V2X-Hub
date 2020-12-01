
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

 #ifndef TMX_PLUGINS_ODELoggerPlugin_H_
 #define TMX_PLUGINS_ODELoggerPlugin_H_



 #include "PluginClient.h"
 #include "PluginDataMonitor.h"

 #include <atomic>
 #include <thread>
 //#include <DecodedBsmMessage.h>
 #include <tmx/messages/IvpJ2735.h>
 #include <tmx/j2735_messages/BasicSafetyMessage.hpp>
 #include <tmx/j2735_messages/SpatMessage.hpp>
 #include <BasicSafetyMessage.h>
 #include <tmx/messages/auto_message.hpp>
 #include <rapidjson/document.h>
 #include <rapidjson/writer.h>
 #include <rapidjson/stringbuffer.h>
 #include <librdkafka/rdkafkacpp.h>

 using namespace std;
 using namespace tmx;
 using namespace tmx::utils;
 using namespace tmx::messages;
 using namespace rapidjson;

 namespace ODELoggerPlugin
 {


 #define BYTESTOMB 1048576

 /**
  * This plugin logs the BSM messages received in the following CSV format.
  */
 class ODELoggerPlugin: public PluginClient
 {
 public:
 	ODELoggerPlugin(std::string);
 	virtual ~ODELoggerPlugin();
 	int Main();
 protected:
 	void UpdateConfigSettings();

 	// Virtual method overrides.
 	void OnConfigChanged(const char *key, const char *value);
 	void OnStateChange(IvpPluginState state);

 	void HandleRealTimePublish(BsmMessage &msg, routeable_message &routeableMsg);
 	void HandleRealTimePublish(SpatMessage &msg, routeable_message &routeableMsg);

 	void GetInt32(unsigned char *buf, int32_t *value)
 	{
 		*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
 	}
 private:
 	std::atomic<uint64_t> _frequency{0};
 	DATA_MONITOR(_frequency);   // Declares the

 	//void OpenBSMLogFile();
 	//void CheckBSMLogFileSizeAndRename(bool createNewFile=false);
 	void QueueKafkaMessage(RdKafka::Producer *producer, std::string topic, std::string message);

 	//std::string  GetCurDateTimeStr();

 	int _scheduleFrequency;
 	bool _forwardBSM;
 	std::string _BSMkafkaTopic;
 	std::string _kafkaBrokerIp;
 	std::string _kafkaBrokerPort;
 	std::string kafkaConnectString;
 	RdKafka::Conf *kafka_conf;
 	RdKafka::Producer *kafka_producer;
 };
 std::mutex _cfgLock;


 } /* namespace ODELoggerPlugin */

 #endif /* ODELoggerPlugin.h */
