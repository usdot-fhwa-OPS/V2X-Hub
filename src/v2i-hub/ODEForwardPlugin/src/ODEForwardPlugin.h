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

#include <PluginClient.h>
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <atomic>
#include <thread>
#include <map>
#include <mutex>
#include <memory>
#include <boost/algorithm/string.hpp>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <BasicSafetyMessage.h>
#include <tmx/messages/auto_message.hpp>
#include <tmx/json/cJSON.h>
#include <environment/EnvUtils.h>
#include <kafka/kafka_client.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "UDPMessageForwarder.h"
#include "CTI4501ValidationMessage.h"

 


namespace ODEForwardPlugin
{

	/**
	 * This plugin logs the BSM messages received in the following CSV format.
	 */
	class ODEForwardPlugin: public tmx::utils::PluginClient
	{
		public:
			explicit ODEForwardPlugin(const std::string &name);
			~ODEForwardPlugin() override = default;
		protected:
			void UpdateConfigSettings();

			// Create the Kafka producer once at registration (called from OnStateChange),
			// after UpdateConfigSettings has populated _kafkaBrokers.
			void InitKafkaProducer();

			// Virtual method overrides.
			void OnConfigChanged(const char *key, const char *value) override;
			void OnStateChange(IvpPluginState state) override;

			void HandleRealTimePublish(tmx::messages::BsmMessage &msg, tmx::routeable_message &routeableMsg);
			void HandleSPaTPublish(tmx::messages::SpatMessage &msg, tmx::routeable_message &routeableMsg);
			void HandleTimPublish(tmx::messages::TimMessage &msg, tmx::routeable_message &routeableMsg);
			void HandleMapPublish(tmx::messages::MapDataMessage &msg, tmx::routeable_message &routeableMsg);

			/**
			 * Handle a CTI 4501 validation event emitted by the IntersectionValidationPlugin
			 * and forward its JSON payload to the matching jpo-conflictmonitor Kafka topic.
			 */
			void HandleValidationEvent(tmx::messages::CTI4501ValidationMessage &msg, tmx::routeable_message &routeableMsg);

		private:

			void sendUDPMessage(tmx::routeable_message &routeableMsg, UDPMessageType udpMessageType) const;

			void toNumber(rapidjson::Value &parent, const std::vector<std::string> &fields) const;

			std::string convertToNum(const std::string &json) const;


			// Forwarded/skipped counters for one message stream
			struct ForwardStats
			{
				uint forwarded = 0;
				uint skipped = 0;
			};
			ForwardStats _bsmStats;
			ForwardStats _spatStats;
			ForwardStats _timStats;
			ForwardStats _mapStats;
			ForwardStats _validationStats;

			std::shared_ptr<UDPMessageForwarder> _udpMessageForwarder;

			// Kafka validation-event forwarding

			/// Kafka broker connection string (e.g. "localhost:9092"). Empty disables forwarding.
			std::string _kafkaBrokers;
			/// Shared producer; fans out to per-event-type topics via send(payload, topic).
			std::shared_ptr<tmx::utils::kafka_producer_worker> _kafkaProducer;
			/// Maps the validation eventType string to its destination Kafka topic.
			std::map<std::string, std::string, std::less<>> _validationTopics;
			/// Serializes producer (re)creation and send(); send() recreates the topic
			/// handle on a topic-name change, which is not concurrency-safe on its own.
			std::mutex _kafkaLock;

	};


} /* namespace ODEForwardPlugin */

#endif /* ODEForwardPlugin.h */