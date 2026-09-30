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


#include "ODEForwardPlugin.h"
using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
namespace ODEForwardPlugin
{
			
 /**
  * Construct a new ODEForwardPlugin with the given name.
  *
  * @param name The name to give the plugin for identification purposes.
  */
	ODEForwardPlugin::ODEForwardPlugin(const string &name): PluginClient(name)
	{
		AddMessageFilter < BsmMessage > (this, &ODEForwardPlugin::HandleRealTimePublish);
		AddMessageFilter < SpatMessage > (this, &ODEForwardPlugin::HandleSPaTPublish);
		AddMessageFilter < MapDataMessage > (this, &ODEForwardPlugin::HandleMapPublish);
		AddMessageFilter < TimMessage > (this, &ODEForwardPlugin::HandleTimPublish);

		AddMessageFilter < CTI4501ValidationMessage > (this, &ODEForwardPlugin::HandleValidationEvent);

		// Subscribe to all messages specified by the filters above.
		SubscribeToMessages();
		_udpMessageForwarder = std::make_shared<UDPMessageForwarder>();
	}


	/**
	 * Updates configuration settings
	 */
	void ODEForwardPlugin::UpdateConfigSettings()
	{
		// Configuration settings are retrieved from the API using the GetConfigValue template class.
		// This method does NOT execute in the main thread, so variables must be protected
		// (e.g. using std::atomic, std::mutex, etc.).

		int instance;
		GetConfigValue("Instance", instance);
		GetConfigValue<uint16_t>("schedule_frequency", _scheduleFrequency);
		GetConfigValue<uint16_t>("ForwardMSG", _forwardMSG);
		GetConfigValue<int>("MAPUDPPort", _MAPUDPPort);
		GetConfigValue<int>("TIMUDPPort", _TIMUDPPort);
		GetConfigValue<int>("BSMUDPPort", _BSMUDPPort);
		GetConfigValue<int>("SPATUDPPort", _SPATUDPPort);
		_udpServerIpAddress = tmx::utils::environment::get_local_ip();
		//Update communication mode

		

		//Create UDP clients for different messages
		_udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>(_udpServerIpAddress, _BSMUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>(_udpServerIpAddress, _MAPUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>(_udpServerIpAddress, _TIMUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>(_udpServerIpAddress, _SPATUDPPort));

		// Kafka validation-event forwarding configuration
		// Kafka validation-event forwarding configuration
		std::string kafkaBrokerIp;
		std::string kafkaBrokerPort;
		GetConfigValue<std::string>("KafkaBrokerIp", kafkaBrokerIp);
		GetConfigValue<std::string>("KafkaBrokerPort", kafkaBrokerPort);

		std::scoped_lock kafkaLock(_kafkaLock);

		if (!kafkaBrokerIp.empty() && !kafkaBrokerPort.empty())
		{
			_kafkaBrokers = kafkaBrokerIp + ":" + kafkaBrokerPort;
		}
		else
		{
			_kafkaBrokers.clear();
		}

		// jpo-conflictmonitor topic names
		_validationTopics = {
			{"SpatMinimumData", "topic.CmSpatMinimumDataEvents"},
			{"MapMinimumData", "topic.CmMapMinimumDataEvents"},
			{"SpatMessageCountProgression", "topic.CmSpatMessageCountProgressionEvents"},
			{"MapMessageCountProgression", "topic.CmMapMessageCountProgressionEvents"},
			{"SpatBroadcastRate", "topic.CmSpatBroadcastRateEvents"},
			{"MapBroadcastRate", "topic.CmMapBroadcastRateEvents"},
		};
	}

	/**
	 * Create the Kafka producer used to forward CTI 4501 validation events
	 */
	void ODEForwardPlugin::InitKafkaProducer()
	{
		std::scoped_lock kafkaLock(_kafkaLock);

		if (_kafkaBrokers.empty())
		{
			PLOG(logWARNING) << "KafkaBrokers not configured; CTI 4501 validation-event "
			                    "forwarding to conflictmonitor is disabled.";
			return;
		}

		kafka_client client;
		_kafkaProducer = client.create_producer(_kafkaBrokers);
		if (!_kafkaProducer || !_kafkaProducer->init_producer())
		{
			PLOG(logERROR) << "Failed to initialize Kafka producer for brokers '"
			               << _kafkaBrokers << "'; validation-event forwarding disabled.";
			_kafkaProducer.reset();
			return;
		}

		PLOG(logINFO) << "Kafka producer initialized for validation-event "
		                 "forwarding to brokers '" << _kafkaBrokers << "'.";
	}

	/**
	 * Called when configuration is changed
	 *
	 * @param key Key of the configuration value changed
	 * @param value Changed value
	 */
	void ODEForwardPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClient::OnConfigChanged(key, value);
		UpdateConfigSettings();
	}

	/**
	 * Called on plugin state change
	 *
	 * @para state New plugin state
	 */
	void ODEForwardPlugin::OnStateChange(IvpPluginState state)
	{
		PluginClient::OnStateChange(state);

		if (state == IvpPluginState_registered)
		{
			UpdateConfigSettings();
			InitKafkaProducer();
		}
	}

	/**
	 * Method that's called to process a message that this plugin has
	 * subscribed for.  This particular method decodes the BSM message and
	 * logs selective fields to a log file.
	 *
	 * @param msg BSMMessage that is received
	 * @routeable_message not used
	 */
	void ODEForwardPlugin::HandleRealTimePublish([[maybe_unused]] BsmMessage &msg, routeable_message &routeableMsg) {
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::BSM);
			++_bsmStats.forwarded;
			SetStatus<uint>("BSM Forwarded", _bsmStats.forwarded);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward BSM message: " << e.what();
			++_bsmStats.skipped;
			SetStatus<uint>("BSM Skipped", _bsmStats.skipped);
		}

	}

	void ODEForwardPlugin::HandleSPaTPublish([[maybe_unused]] SpatMessage &msg, routeable_message &routeableMsg) {
		PLOG(logDEBUG) << "ODE HandleSPaT flags=" << routeableMsg.get_flags();
		if (!(routeableMsg.get_flags() & IvpMsgFlags_Validated)) {
			PLOG(logDEBUG) << "ODE skip, not validated";
        	return;
		}
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::SPAT);
			++_spatStats.forwarded;
			SetStatus<uint>("SPAT Forwarded", _spatStats.forwarded);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward SPAT message: " << e.what();
			++_spatStats.skipped;
			SetStatus<uint>("SPAT Skipped", _spatStats.skipped);
		}
	}

	void ODEForwardPlugin::HandleTimPublish([[maybe_unused]] TimMessage &msg, routeable_message &routeableMsg) {
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::TIM);
			++_timStats.forwarded;
			SetStatus<uint>("TIM Forwarded", _timStats.forwarded);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward TIM message: " << e.what();
			++_timStats.skipped;
			SetStatus<uint>("TIM Skipped", _timStats.skipped);
		}
	}


	void ODEForwardPlugin::HandleMapPublish([[maybe_unused]] MapDataMessage &msg, routeable_message &routeableMsg) {
		PLOG(logDEBUG) << "ODE HandleMAP flags=" << routeableMsg.get_flags();
		if (!(routeableMsg.get_flags() & IvpMsgFlags_Validated)) {
			PLOG(logDEBUG) << "ODE skip, not validated";
        	return;
		}
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::MAP);
			++_mapStats.forwarded;
			SetStatus<uint>("MAP Forwarded", _mapStats.forwarded);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward MAP message: " << e.what();
			++_mapStats.skipped;
			SetStatus<uint>("MAP Skipped", _mapStats.skipped);
		}
	}

	void ODEForwardPlugin::HandleValidationEvent(CTI4501ValidationMessage &msg, routeable_message &routeableMsg) {
		const std::string eventType = msg.get_eventType();

		std::scoped_lock lock(_kafkaLock);

		if (!_kafkaProducer || !_kafkaProducer->is_running())
		{
			PLOG(logWARNING) << "Dropping validation event '" << eventType
			                 << "': Kafka producer is not available.";
			++_validationStats.skipped;
			SetStatus<uint>("Validation Events Skipped", _validationStats.skipped);
			return;
		}

		auto it = _validationTopics.find(eventType);
		if (it == _validationTopics.end())
		{
			PLOG(logWARNING) << "Dropping validation event: no Kafka topic configured for "
			                    "eventType '" << eventType << "'.";
			++_validationStats.skipped;
			SetStatus<uint>("Validation Events Skipped", _validationStats.skipped);
			return;
		}

		try {
			// Forward the payload unchanged
			const std::string payload = routeableMsg.get_payload_str();
			_kafkaProducer->send(payload, it->second);
			PLOG(logDEBUG) << "Forwarded validation event '" << eventType
			               << "' to Kafka topic '" << it->second << "'.";
			++_validationStats.forwarded;
			SetStatus<uint>("Validation Events Forwarded", _validationStats.forwarded);
		} catch (const std::exception &e) {
			PLOG(logERROR) << "Failed to forward validation event '" << eventType
			               << "': " << e.what();
			++_validationStats.skipped;
			SetStatus<uint>("Validation Events Skipped", _validationStats.skipped);
		}
	}

	void ODEForwardPlugin::sendUDPMessage(routeable_message &routeableMsg, UDPMessageType udpMessageType) const{
		std::string message = routeableMsg.get_payload_str().c_str();
		PLOG(logDEBUG) << "Sending UDP Message: " << message;
		_udpMessageForwarder->sendMessage(udpMessageType, message);
	}


} /* namespace ODEForwardPlugin */


/**
 * Main method for running the plugin
 * @param argc number of arguments
 * @param argv array of arguments
 */
int main(int argc, char *argv[])
{
	return tmx::utils::run_plugin<ODEForwardPlugin::ODEForwardPlugin>("ODEForwardPlugin", argc, argv);
}