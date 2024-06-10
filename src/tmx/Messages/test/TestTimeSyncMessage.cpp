#include <gtest/gtest.h>
#include "TimeSyncMessage.h"
namespace tmx::messages {

    TEST(TestTimeSyncMessage, to_string) {
        TimeSyncMessage msg(20, 30);
        std::string json = "{ \"timestep\":20, \"seq\":30}";
        ASSERT_EQ( json, msg.to_string());
    }
}