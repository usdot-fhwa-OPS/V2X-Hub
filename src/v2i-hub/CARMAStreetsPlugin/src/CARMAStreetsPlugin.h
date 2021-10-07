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

private:
	std::string _receiveTopic;
	std::string _transmitMobilityOperationTopic;
	std::string _transmitMobilityPathTopic;
	std::string _kafkaBrokerIp;
	std::string _kafkaBrokerPort;
	RdKafka::Conf *kafka_conf;
 	RdKafka::Producer *kafka_producer;
	RdKafka::Consumer *kafka_consumer;
	std::vector<std::string> _strategies;
};
std::mutex _cfgLock;

}
#endif