# GMock Guide: Mocking PluginClient for V2X Hub Plugin Unit Testing

## Overview

This guide explains how to create a GMock (Google Mock) for the `PluginClient` base class to enable unit testing of V2X Hub plugins without requiring the full plugin infrastructure.

## Why Mock PluginClient?

`PluginClient` has several dependencies that are difficult or impossible to instantiate in unit tests:
- IVP plugin system initialization
- Database connections
- System configuration
- Message routing infrastructure
- Internal static state management

By creating a mock, you can isolate your plugin logic and test it without these dependencies.

## Key Concepts

### 1. Virtual Methods in PluginClient

The methods you'll typically want to mock are the virtual ones:

```cpp
// From PluginClient.h
virtual int Main();
virtual bool ProcessOptions(const boost::program_options::variables_map &);
virtual void OnConfigChanged(const char *key, const char *value);
virtual void OnError(IvpError err);
virtual void OnMessageReceived(IvpMessage *msg);
virtual void OnStateChange(IvpPluginState state);
```

Additionally, these utility methods are useful to mock:
- `BroadcastMessage()` - Prevent actual message routing
- `GetConfigValue()` - Return test configuration values
- `SetStatus()` - Capture status updates for verification
- `get_DbConnection()` - Prevent database access

### 2. Non-Virtual Methods and Static Members

Some challenges with mocking `PluginClient`:
- **Constructor**: Calls `ivp_create()` internally - this will fail in unit tests
- **Static members**: `_instanceMap`, `_sysContext` are static and may cause issues
- **Non-virtual methods**: `BroadcastMessage()` has both virtual and non-virtual overloads

## Solution: Create a MockPluginClient

### Step 1: Design Your Test Mock Header

Create a mock class that inherits from `PluginClient`:

```cpp
// File: tests/unit/mocks/MockPluginClient.h

#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "PluginClient.h"

namespace tmx {
namespace utils {
namespace test {

/**
 * MockPluginClient provides a testable base class for unit testing plugins.
 * It overrides problematic constructor behavior and provides mock methods.
 */
class MockPluginClient : public PluginClient {
public:
    // We need to handle the constructor carefully since PluginClient's 
    // constructor calls ivp_create() which requires the IVP system
    
    MockPluginClient(const std::string &name = "MockPlugin") 
        : PluginClient(name) {
        // At this point, PluginClient has already called ivp_create()
        // In a real testing scenario, you may need to wrap/mock this separately
    }
    
    virtual ~MockPluginClient() = default;

    // Mock the virtual methods
    MOCK_METHOD(int, Main, (), (override));
    MOCK_METHOD(bool, ProcessOptions, 
                (const boost::program_options::variables_map &), (override));
    MOCK_METHOD(void, OnConfigChanged, 
                (const char *key, const char *value), (override));
    MOCK_METHOD(void, OnError, (IvpError err), (override));
    MOCK_METHOD(void, OnMessageReceived, (IvpMessage *msg), (override));
    MOCK_METHOD(void, OnStateChange, (IvpPluginState state), (override));

    // Mock utility methods that are hard to test with
    MOCK_METHOD(std::string, GetName, (), (override));
    
    // Helper to mock BroadcastMessage - template method variations
    // Note: BroadcastMessage is not virtual, so we provide a hook instead
    std::function<void(const tmx::routeable_message &)> broadcast_hook;
    
    // Helper method for testing
    void SetBroadcastHook(
        std::function<void(const tmx::routeable_message &)> hook) {
        broadcast_hook = hook;
    }
};

} // namespace test
} // namespace utils
} // namespace tmx
```

### Step 2: Alternative - Better Approach with Constructor Wrapping

For more control, wrap the problematic constructor:

```cpp
// File: tests/unit/mocks/TestablePluginClient.h

#pragma once

#include <gmock/gmock.h>
#include "PluginClient.h"

namespace tmx {
namespace utils {
namespace test {

/**
 * TestablePluginClient provides a way to test plugin logic without
 * the full PluginClient initialization. This class exists to allow
 * custom construction logic in tests.
 */
class TestablePluginClient : public PluginClient {
public:
    // Mock virtual methods
    MOCK_METHOD(int, Main, (), (override));
    MOCK_METHOD(bool, ProcessOptions, 
                (const boost::program_options::variables_map &opts), (override));
    MOCK_METHOD(void, OnConfigChanged, 
                (const char *key, const char *value), (override));
    MOCK_METHOD(void, OnError, (IvpError err), (override));
    MOCK_METHOD(void, OnMessageReceived, (IvpMessage *msg), (override));
    MOCK_METHOD(void, OnStateChange, (IvpPluginState state), (override));
    
    // If PluginClient had more virtual methods, mock them here
    
    // For template methods and non-virtual methods, provide test hooks
    struct BroadcastCapture {
        tmx::routeable_message message;
        std::string source;
        unsigned int sourceId;
        IvpMsgFlags flags;
    };
    
    std::vector<BroadcastCapture> broadcast_messages;
    
    void ClearBroadcastMessages() {
        broadcast_messages.clear();
    }
    
    // Override non-virtual BroadcastMessage to capture calls
    void BroadcastMessage(const tmx::routeable_message &routeableMsg) {
        BroadcastCapture capture{routeableMsg, "", 0, IvpMsgFlags_None};
        broadcast_messages.push_back(capture);
    }
};

} // namespace test
} // namespace utils
} // namespace tmx
```

## Step 3: Create a Plugin Test Fixture

### Example: Testing a Child Plugin Class

Assume you have a plugin like this:

```cpp
// File: src/v2i-hub/ExamplePlugin/src/ExamplePlugin.h

class ExamplePlugin : public tmx::utils::PluginClient {
public:
    ExamplePlugin(std::string name) : PluginClient(name) {}
    virtual ~ExamplePlugin() = default;
    
    virtual int Main() override;
    virtual void OnMessageReceived(IvpMessage *msg) override;
    
    // Application-specific methods to test
    void ProcessIncomingData(const std::string &data);
    std::string GetProcessedResult();

private:
    std::string _result;
};
```

Create a unit test:

```cpp
// File: tests/unit/ExamplePluginTest.cpp

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ExamplePlugin.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Invoke;

namespace {

/**
 * Test fixture for ExamplePlugin.
 * This fixture creates a mock PluginClient and injects it into the plugin.
 */
class ExamplePluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin = std::make_unique<ExamplePlugin>("TestPlugin");
    }
    
    void TearDown() override {
        plugin.reset();
    }
    
    std::unique_ptr<ExamplePlugin> plugin;
};

// Test basic message processing
TEST_F(ExamplePluginTest, ProcessesIncomingDataCorrectly) {
    // Arrange
    std::string testData = "test input";
    
    // Act
    plugin->ProcessIncomingData(testData);
    
    // Assert
    EXPECT_EQ("processed: test input", plugin->GetProcessedResult());
}

// Test message format validation
TEST_F(ExamplePluginTest, RejectsInvalidMessageFormat) {
    // Arrange
    std::string invalidData = "";
    
    // Act
    plugin->ProcessIncomingData(invalidData);
    
    // Assert
    EXPECT_EQ("", plugin->GetProcessedResult());
}

} // namespace
```

## Step 4: Mock PluginClient Dependencies

For more advanced testing scenarios, create a mock factory:

```cpp
// File: tests/unit/mocks/PluginClientMockFactory.h

#pragma once

#include <gmock/gmock.h>
#include <memory>
#include "PluginClient.h"

namespace tmx {
namespace utils {
namespace test {

class MockPluginClientDependencies {
public:
    // Mock database connection
    MOCK_METHOD(tmx::utils::DbConnection, get_DbConnection, ());
    
    // Mock configuration retrieval
    MOCK_METHOD(bool, GetConfigValue, 
                (const std::string &key, std::string &value, std::mutex *lock),
                (const));
    
    // Mock status setting
    MOCK_METHOD(bool, SetStatus, 
                (const char *key, const std::string &value));
};

/**
 * Factory to create properly configured mock PluginClient instances
 */
class PluginClientMockFactory {
public:
    static std::shared_ptr<MockPluginClientDependencies> 
    CreateMockDependencies() {
        auto mock = std::make_shared<MockPluginClientDependencies>();
        
        // Set default expectations
        EXPECT_CALL(*mock, get_DbConnection())
            .WillRepeatedly(::testing::Return(
                tmx::utils::DbConnection("", "", "", "")));
        
        return mock;
    }
};

} // namespace test
} // namespace utils
} // namespace tmx
```

## Step 5: Complete Example - Testing a Plugin with Message Handling

```cpp
// File: tests/unit/plugins/RtcmPluginTest.cpp

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "RtcmPlugin.h"
#include "mocks/MockPluginClient.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::AtLeast;

namespace RtcmPlugin {
namespace test {

class RtcmPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin_ = std::make_unique<RtcmPlugin>("TestRtcmPlugin");
    }
    
    std::unique_ptr<RtcmPlugin> plugin_;
};

// Test RTCM message processing
TEST_F(RtcmPluginTest, ProcessesRtcmMessageCorrectly) {
    // This test verifies that the plugin correctly processes RTCM messages
    // without requiring the full IVP infrastructure
    
    // Arrange
    std::vector<uint8_t> rtcmData = {/* RTCM binary data */};
    
    // Act
    // Call the method that processes RTCM data
    plugin_->ProcessRtcmData(rtcmData);
    
    // Assert
    // Verify the plugin's internal state was updated correctly
    EXPECT_TRUE(plugin_->HasProcessedData());
}

// Test configuration handling
TEST_F(RtcmPluginTest, HandlesConfigurationChanges) {
    // Arrange
    const char *key = "EnableLogging";
    const char *value = "true";
    
    // Act
    plugin_->OnConfigChanged(key, value);
    
    // Assert
    bool loggingEnabled = false;
    plugin_->GetConfigValue(std::string(key), loggingEnabled);
    EXPECT_TRUE(loggingEnabled);
}

// Test state transitions
TEST_F(RtcmPluginTest, HandlesStateTransitions) {
    // Arrange
    // Act
    plugin_->OnStateChange(IVP_STATUS_RUNNING);
    
    // Assert
    EXPECT_EQ(IVP_STATUS_RUNNING, plugin_->GetCurrentState());
}

// Test message broadcasting (mocked)
TEST_F(RtcmPluginTest, BroadcastsProcessedMessages) {
    // Arrange
    std::vector<uint8_t> rtcmData = {/* RTCM data */};
    
    // Mock the broadcast mechanism
    int broadcast_count = 0;
    auto capture_broadcast = [&](const tmx::routeable_message &msg) {
        broadcast_count++;
    };
    
    // Act
    plugin_->ProcessRtcmData(rtcmData);
    
    // Assert - verify message was broadcast
    EXPECT_GT(broadcast_count, 0);
}

} // namespace test
} // namespace RtcmPlugin
```

## Best Practices

### 1. **Isolate Plugin Logic**
- Mock PluginClient to test only the plugin's business logic
- Don't test the PluginClient framework itself - that's already tested

### 2. **Mock Constructor Issues**
```cpp
// If PluginClient constructor is problematic, consider this pattern:
class TestablePluginChild : public YourPlugin {
public:
    // Allow tests to skip full initialization
    TestablePluginChild() : YourPlugin("test") {
        // Can add test-specific setup here
    }
};
```

### 3. **Use Matchers for Complex Types**
```cpp
// For IvpMessage* testing, create custom matchers
MATCHER_P(IvpMessageOfType, type, "") {
    return arg && arg->type && strcmp(arg->type, type) == 0;
}

// Usage:
EXPECT_CALL(mock_plugin, OnMessageReceived(IvpMessageOfType("BSM")));
```

### 4. **Capture and Inspect Calls**
```cpp
tmx::routeable_message captured_message;
EXPECT_CALL(plugin, BroadcastMessage(_))
    .WillOnce(Invoke([&](const tmx::routeable_message &msg) {
        captured_message = msg;
    }));

plugin.SendData();
EXPECT_EQ("ExpectedType", captured_message.get_type());
```

### 5. **Test State Management**
```cpp
TEST_F(PluginTest, MaintainsStateAcrossCalls) {
    plugin->OnConfigChanged("key1", "value1");
    plugin->OnConfigChanged("key2", "value2");
    
    // Verify both values are retained
    std::string val1, val2;
    EXPECT_TRUE(plugin->GetConfigValue("key1", val1));
    EXPECT_TRUE(plugin->GetConfigValue("key2", val2));
}
```

## Running Tests

```bash
# From your test directory
cd tests/unit

# Build and run
cmake ..
make
ctest --verbose

# Or directly run a test
./YourPluginTest
```

## Troubleshooting

### Issue: "undefined reference to PluginClient::..."
**Solution**: Ensure you're linking against the PluginClient library in your test CMakeLists.txt:
```cmake
target_link_libraries(YourPluginTest 
    PUBLIC tmx-utils
    PRIVATE gtest gmock
)
```

### Issue: Static initialization order problems
**Solution**: Mock the static methods:
```cpp
// In your test setup
EXPECT_CALL(MockPluginClient::_instanceMap)
    .WillRepeatedly(Return(std::map<IvpPlugin*, PluginClient*>()));
```

### Issue: IVP system calls fail
**Solution**: Wrap the initialization in try-catch in tests or use a test harness that provides minimal IVP stubs.

## References

- [Google Test (gtest) Documentation](https://github.com/google/googletest)
- [Google Mock (gmock) Documentation](https://github.com/google/googletest/tree/main/googlemock)
- [V2X Hub PluginClient.h](/src/tmx/TmxUtils/src/PluginClient.h)

## Example Project Structure

```
tests/
├── unit/
│   ├── CMakeLists.txt
│   ├── mocks/
│   │   ├── MockPluginClient.h
│   │   ├── TestablePluginClient.h
│   │   └── PluginClientMockFactory.h
│   ├── plugins/
│   │   ├── RtcmPluginTest.cpp
│   │   ├── CommandPluginTest.cpp
│   │   └── ExamplePluginTest.cpp
│   └── README.md
└── integration/
    └── ...
```

---

**Version**: 1.0  
**Last Updated**: November 2025  
**Maintainer**: V2X Hub Development Team
