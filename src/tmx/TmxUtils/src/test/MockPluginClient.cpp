/*
 * MockPluginClient.cpp
 * 
 * Implementation of MockPluginClient for unit testing V2X Hub plugins.
 */

#include "MockPluginClient.h"

namespace tmx {
namespace utils {
namespace test {

MockPluginClient::MockPluginClient(const std::string &name) {
    // Constructor calls parent PluginClient which initializes _plugin via ivp_create()
    // This will work in unit test environments if the IVP mock/stub is available
}

MockPluginClient::~MockPluginClient() = default;

} // namespace test
} // namespace utils
} // namespace tmx
