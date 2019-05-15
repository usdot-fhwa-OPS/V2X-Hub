#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"

#include <atomic>
#include <thread>
#include <DecodedBsmMessage.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>

#include <UdpClient.h>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;


namespace PreemptionPlugin
{

/**
 * This plugin is an example to demonstrate the capabilities of a TMX plugin.
 */
class PreemptionPlugin: public PluginClient
{
public:
	PreemptionPlugin(std::string);
	virtual ~PreemptionPlugin();
	int Main();

protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

	void HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	void HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg);
	void HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg);
private:
	std::atomic<uint64_t> _frequency{0};
	DATA_MONITOR(_frequency);

	const char *ipwithport = "192.168.55.49:6053";
	int snmp_version = SNMP_VERSION_1;
	const char *snmp_community = "public";
	std::string BasePreemptionOid = ".1.3.6.1.4.1.1206.4.2.1.6.3.1.2.";
	std::string PreemptionPlan;
	const char *PreemptionPlan_flag;

	// sends oid to controler
	int SendOid(const char *PreemptionOid, const char *value);
	tmx::utils::UdpClient *_signSimClient = NULL;
};
};