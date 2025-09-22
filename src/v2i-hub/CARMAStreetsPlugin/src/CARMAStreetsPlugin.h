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
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "jsoncpp/json/json.h"
#include <pthread.h>
#include <boost/thread.hpp>
#include <mutex>
#include "J2735MapToJsonConverter.h"
#include "JsonToJ2735SpatConverter.h"
#include "J2735ToSRMJsonConverter.h"   
#include <kafka/kafka_client.h>
#include <kafka/kafka_consumer_worker.h>
#include "JsonToJ2735SSMConverter.h"
#include <SensorDetectedObject.h>
#include "JsonToJ3224SDSMConverter.h"
#include "J3224ToSDSMJsonConverter.h"
#include "PluginClientClockAware.h"




using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace boost::property_tree;

namespace CARMAStreetsPlugin {

class CARMAStreetsPlugin: public PluginClientClockAware {
public:
	CARMAStreetsPlugin(std::string);
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);

	void OnStateChange(IvpPluginState state);
	void HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg);
	void HandleMobilityPathMessage(tsm2Message &msg, routeable_message &routeableMsg);
	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);
	/**
	 * @brief Callback function when the plugin received detected object, and forward the detected object to Kafka topic.
	 * @param msg Detected object received from TMX bus.
	 * @param routeableMsg routeable_message for detected object.
	 */
	void HandleSimulatedSensorDetectedMessage(SensorDetectedObject &msg,  routeable_message &routeableMsg);
	/**
	 * @brief Overide PluginClientClockAware HandleTimeSyncMessage to producer TimeSyncMessage to kafka for CARMA Streets Time Synchronization.
	 * @param msg TimeSyncMessage received by plugin when in simulation mode. Message provides current simulation time to all processes.
	 * @param routeableMsg routeable_message for time sync message.
	 */
	void HandleTimeSyncMessage(TimeSyncMessage &msg, routeable_message &routeableMsg) override;
	/**
	 * @brief Subscribe to MAP message broadcast by the MAPPlugin. This handler will be called automatically whenever the MAPPlugin is broadcasting a J2735 MAP message.
	 * @param msg The J2735 MAP message received from the internal 
	 * @param routeableMsg 
	 */
	void HandleMapMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	/**
	 * @brief Subscribes to incoming ASN.1, C-Struct formatted SDSMs generated from broadcasting RSUs. These SDSM C-Structs are then converted to JSON to be forwarded to CARMA Streets/Kafka by the handler.
	 * @param msg The J3224 SDSM received from the internal 
	 * @param routeableMsg 
	 */
	void HandleSDSMMessage(SdsmMessage &msg, routeable_message &routeableMsg);
	/**
	 * @brief Subscribe to SRM message received from RSU and publish the message to a Kafka topic
	*/
	void HandleSRMMessage (SrmMessage &msg, routeable_message &routeableMsg);
	/**
	 * @brief Subcribe to scheduling plan Kafka topic created by carma-streets
	 */
	void SubscribeSchedulingPlanKafkaTopic();
	/**
	 * @brief Subcribe to SPAT Kafka topic created by carma-streets
	 */
	void SubscribeSpatKafkaTopic();
	/**
	 * @brief Subcribe to SSM Kafka topic created by carma-streets
	 */
	void SubscribeSSMKafkaTopic();
	/**
	 * @brief Subcribe to SDSM Kafka topic created by carma-streets
	 */
	void SubscribeSDSMKafkaTopic();

	bool getEncodedtsm3(tsm3EncodedMessage *tsm3EncodedMsg,  Json::Value metadata, Json::Value payload_json);
	/**
	 * @brief Produce message to a kafka topic
	 * @param msg Json format message to send to a topic
	 * @param topic_name The name of the topic
	 */
	void produce_kafka_msg(const string &msg, const string &topic_name) const;
	/**
	 * @brief Initialize Kafka Producers and Consumers
	*/
	void InitKafkaConsumerProducers();
	

private:
	tmx::messages::J2735MessageFactory factory;
	std::string _receiveTopic;
	std::string _transmitMobilityOperationTopic;
	std::string _subscribeToSchedulingPlanTopic;
	std::string _subscribeToSchedulingPlanConsumerGroupId;
	std::string _subscribeToSpatTopic;
	std::string _subscribeToSsmTopic;
	std::string _subscribeToSdsmTopic;
	std::string _subscribeToSpatConsumerGroupId;
	std::string _subscribeToSSMConsumerGroupId;
	std::string _transmitMobilityPathTopic;
	std::string _transmitBSMTopic;
	std::string _transmitMAPTopic;
	std::string _transmitSRMTopic;
	std::string _transmitSimSensorDetectedObjTopic;
	std::string _transmitSDSMTopic;
	std::string _kafkaBrokerIp;
	std::string _kafkaBrokerPort;
	std::shared_ptr<kafka_producer_worker> _kafka_producer_ptr;
	std::shared_ptr<kafka_consumer_worker> _spat_kafka_consumer_ptr;
	std::shared_ptr<kafka_consumer_worker> _scheduing_plan_kafka_consumer_ptr;
	std::shared_ptr<kafka_consumer_worker> _ssm_kafka_consumer_ptr;
	std::shared_ptr<kafka_consumer_worker> _sdsm_kafka_consumer_ptr;
	std::vector<std::string> _strategies;
	tmx::messages::tsm3Message *_tsm3Message{NULL};
	std::mutex data_lock;

	/**
	 * @brief Status label for SPAT messages skipped due to errors.
	 */
	const char* Key_SPATMessageSkipped = "SPAT messages skipped due to errors.";
	/**
	 * @brief Count for SPAT messages skipped due to errors.
	 */
	uint _spatMessageSkipped = 0;

	/**
	 * @brief Status label for Intersection Schedule messages skipped due to errors.
	 */
	const char* Key_ScheduleMessageSkipped = "Intersection Schedule messages skipped due to errors.";

	/**
	 * @brief Count for Intersection Schedule messages skipped due to errors.
	 */
	uint _scheduleMessageSkipped = 0;

	/**
	 * @brief Status label for MAP messages skipped due to errors.
	 */
	const char* Key_MAPMessageSkipped = "MAP messages skipped due to errors.";

	/**
	 * @brief Count for MAP messages skipped due to errors.
	 */
	uint _mapMessageSkipped = 0;

	/**
	 * @brief Status label for Mobility Operation messages skipped due to errors.
	 */

	const char* Key_MobilityOperationMessageSkipped = "Mobility Operation messages skipped due to errors.";

	/**
	 * @brief Count for Mobility Operation messages skipped due to errors.
	 */
	uint _mobilityOperationMessageSkipped = 0;

	/**
	 * @brief Status label for Mobility Path messages skipped due to errors.
	 */
	const char* Key_MobilityPathMessageSkipped = "Mobility Path messages skipped due to errors.";

	/**
	 * @brief Count for Mobility Path messages skipped due to errors.
	 */
	uint _mobilityPathMessageSkipped = 0;

	/**
	 * @brief Status label for BSM messages skipped due to  errors.
	 */
	const char* Key_BSMMessageSkipped = "BSM messages skipped due to errors.";

	/**
	 * @brief Count for BSM messages skipped due to errors.
	 */
	uint _bsmMessageSkipped = 0;
	
	/**
	 * @brief Status label for SSM messages skipped due to errors.
	 */
	const char*  Key_SSMMessageSkipped =  "SSM messages skipped due to errors.";

	/**
	 * @brief Count for SSM messages skipped due to errors.
	 */
	uint _ssmMessageSkipped = 0;

	/**
	 * @brief Status label for SDSM messages skipped due to errors.
	 */
	const char*  Key_SDSMMessageSkipped =  "SDSM messages skipped due to errors.";

	/**
	 * @brief Count for SDSM messages skipped due to errors.
	 */
	uint _sdsmMessageSkipped = 0;

	/**
	 * @brief Intersection Id for intersection
	 */
	std::string _intersectionId = "UNSET";
	/**
	 * @brief Status label for SRM messages skipped due to errors.
	 */
	const char* Key_SRMMessageSkipped = "SRM messages skipped due to errors.";
	/**
	 * @brief Count for SRM messages skipped due to errors.
	 */
	uint _srmMessageSkipped = 0;
};
std::mutex _cfgLock;

}
#endif
