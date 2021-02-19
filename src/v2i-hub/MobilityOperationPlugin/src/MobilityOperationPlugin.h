#ifndef SRC_MOBILITYOPERATIONPLUGIN_H_
#define SRC_MOBILITYOPERATIONPLUGIN_H_
#include "PluginClient.h"
#include <tmx/j2735_messages/testMessage03.hpp>


using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace MobilityOperationPlugin {

class MobilityOperationPlugin: public PluginClient {
public:
	MobilityOperationPlugin(std::string);
	virtual ~MobilityOperationPlugin();
	int Main();
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);

	void OnStateChange(IvpPluginState state);

	void HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg);

private:
};
}
#endif