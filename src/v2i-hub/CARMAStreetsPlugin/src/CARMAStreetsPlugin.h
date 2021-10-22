#ifndef SRC_CARMASTREETSPLUGIN_H_
#define SRC_CARMASTREETSPLUGIN_H_
#include "PluginClient.h"
#include <tmx/j2735_messages/testMessage03.hpp>
#include <tmx/j2735_messages/testMessage02.hpp>
#include <librdkafka/rdkafkacpp.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <tmx/TmxException.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include "jsoncpp/json/json.h"
#include <pthread.h>
#include <boost/thread.hpp>
#include <mutex>




using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace boost::property_tree;

namespace CARMAStreetsPlugin {

class CARMAStreetsPlugin: public PluginClient {
public:
	CARMAStreetsPlugin(std::string);
	virtual ~CARMAStreetsPlugin();
	int Main();
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);

	void OnStateChange(IvpPluginState state);
	void HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg);
	void HandleMobilityPathMessage(tsm2Message &msg, routeable_message &routeableMsg);
	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);
	void SubscribeKafkaTopics();
	bool getEncodedtsm3(std::shared_ptr<tsm3EncodedMessage> tsm3EncodedMsg,  std::string msg_payload);

private:
	std::string _receiveTopic;
	std::string _transmitMobilityOperationTopic;
	std::string _subscribeToSchedulingPlanTopic;
	std::string _transmitMobilityPathTopic;
	std::string _transmitBSMTopic;
	std::string _kafkaBrokerIp;
	std::string _kafkaBrokerPort;
	RdKafka::Conf *kafka_conf;
 	RdKafka::Producer *kafka_producer;
	RdKafka::KafkaConsumer *kafka_consumer;
	std::vector<std::string> _strategies;
	tmx::messages::tsm3Message *_tsm3Message{NULL};
	std::mutex data_lock;
		
	/***
	 * Configurable indicator to run consumer and consume messages from kafka topics
	 * run the consumer if it equals = 1; otherwise = 0
	**/
	int _run_kafka_consumer = 0; 
	std::string _intersectionType = "NA";
};
std::mutex _cfgLock;

}
#endif