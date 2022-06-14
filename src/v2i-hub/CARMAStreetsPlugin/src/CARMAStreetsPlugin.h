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
#include <tmx/j2735_messages/MapDataMessage.hpp>
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
	void HandleMapMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	void SubscribeKafkaTopics();
	bool getEncodedtsm3(tsm3EncodedMessage *tsm3EncodedMsg,  Json::Value metadata, Json::Value payload_json);
	void convertJ2735MAPToMapJSON(const std::shared_ptr<MapData> mapMsgPtr,  Json::Value& mapJson) const;
	void produce_kafka_msg(const string& msg, const string& topic_name) const;
	void convertLanesetToJSON(const IntersectionGeometry* intersection, Json::Value& laneSetJson) const;
	void convertLaneAttributeToJSON(const GenericLane * lane, Json::Value& LaneAttributesJson) const;
	void convertNodeListToJSON(const GenericLane* lane , Json::Value& nodeList) const;
	void convertConnectsToJSON(const GenericLane* lane , Json::Value& nodeListJson) const;
	

private:
	std::string _receiveTopic;
	std::string _transmitMobilityOperationTopic;
	std::string _subscribeToSchedulingPlanTopic = "";
	std::string _transmitMobilityPathTopic;
	std::string _transmitBSMTopic;
	std::string _transmitMAPTopic;
	std::string _kafkaBrokerIp;
	std::string _kafkaBrokerPort;
	RdKafka::Conf *kafka_conf;
	RdKafka::Conf *kafka_conf_consumer;
 	RdKafka::Producer *kafka_producer;
	RdKafka::KafkaConsumer *kafka_consumer;
	RdKafka::Topic *_topic;
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