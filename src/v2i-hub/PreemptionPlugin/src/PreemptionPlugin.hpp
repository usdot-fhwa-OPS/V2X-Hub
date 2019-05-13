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
	DATA_MONITOR(_frequency);   // Declares the

	void SendMib(const char *mib, const char *value);
	tmx::utils::UdpClient *_signSimClient = NULL;
};
};