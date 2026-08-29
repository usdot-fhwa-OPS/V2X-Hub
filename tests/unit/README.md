# V2X Hub Plugin Unit Testing with GMock

This directory contains unit tests and mocks for testing V2X Hub plugins without requiring the full plugin infrastructure.

## Quick Start

### Build and Run Tests

```bash
# From the tests/unit directory
mkdir build
cd build
cmake ..
make
ctest --verbose
```

### Or Run Individual Tests

```bash
./example_plugin_test
```

## Directory Structure

```
tests/unit/
├── CMakeLists.txt                 # Build configuration
├── README.md                       # This file
├── mocks/
│   ├── MockPluginClient.h         # Mock base class
│   └── MockPluginClient.cpp       # Mock implementation
├── ExamplePluginTest.cpp          # Example test file
└── plugins/
    ├── RtcmPluginTest.cpp         # RTCM Plugin tests (to be created)
    ├── CommandPluginTest.cpp       # Command Plugin tests (to be created)
    └── ...
```

## Understanding the Mock

### MockPluginClient Class

`MockPluginClient` is a mock implementation of `PluginClient` that:

1. **Inherits from PluginClient** - Allows it to be used wherever PluginClient is expected
2. **Mocks virtual methods** - Enables testing of virtual method behavior
3. **Provides test helpers** - Utility methods to check internal state
4. **Captures messages** - Records broadcast messages for verification

### Key Features

#### 1. Mock Virtual Methods

All virtual methods are automatically mocked:

```cpp
MOCK_METHOD(int, Main, (), (override));
MOCK_METHOD(void, OnConfigChanged, (const char *key, const char *value), (override));
MOCK_METHOD(void, OnMessageReceived, (IvpMessage *msg), (override));
MOCK_METHOD(void, OnStateChange, (IvpPluginState state), (override));
```

#### 2. Message Capture

Enable automatic capturing of broadcast messages:

```cpp
MockPluginClient mock_plugin("TestPlugin");
mock_plugin.EnableMessageCapture();
// ... plugin broadcasts messages ...
auto messages = mock_plugin.GetAllBroadcastMessages();
EXPECT_EQ(1, messages.size());
```

#### 3. Test Helpers

Helper methods for common test operations:

```cpp
mock_plugin.SimulateConfigChange("Key", "Value");
mock_plugin.SimulateStateChange(IVP_STATUS_RUNNING);
auto msg = mock_plugin.GetLastBroadcastMessage();
```

## Creating Tests for Your Plugin

### Step 1: Create a Test File

Create a new test file in `tests/unit/plugins/`:

```cpp
// MyPluginTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MyPlugin.h"

class MyPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin = std::make_unique<MyPlugin>("TestPlugin");
    }
    
    std::unique_ptr<MyPlugin> plugin;
};

TEST_F(MyPluginTest, DoesExpectedBehavior) {
    // Arrange
    // Act
    // Assert
}
```

### Step 2: Add to CMakeLists.txt

```cmake
add_executable(my_plugin_test
    plugins/MyPluginTest.cpp
)

target_link_libraries(my_plugin_test
    plugin_mocks
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    gmock
)

add_test(NAME MyPluginTest COMMAND my_plugin_test)
```

### Step 3: Write Test Cases

```cpp
TEST_F(MyPluginTest, HandlesConfigurationChanges) {
    // Arrange
    const char *key = "EnableFeature";
    const char *value = "true";
    
    // Act
    plugin->OnConfigChanged(key, value);
    
    // Assert
    std::string retrieved_value;
    EXPECT_TRUE(plugin->GetConfigValue(key, retrieved_value));
    EXPECT_EQ("true", retrieved_value);
}

TEST_F(MyPluginTest, BroadcastsMessages) {
    // Arrange
    plugin->EnableMessageCapture();
    
    // Act
    plugin->ProcessData();
    
    // Assert
    EXPECT_GT(plugin->GetAllBroadcastMessages().size(), 0);
}
```

## GMock Matchers for IvpMessage

Custom matchers make it easier to verify message handling:

```cpp
// Match by message type
EXPECT_CALL(plugin, OnMessageReceived(IvpMessageOfType("J2735", "BSM")))
    .Times(1);

// Match by just type
EXPECT_CALL(plugin, OnMessageReceived(IvpMessageHasType("J2735")))
    .Times(AtLeast(1));

// Match by subtype
EXPECT_CALL(plugin, OnMessageReceived(IvpMessageHasSubType("BSM")))
    .Times(AnyNumber());
```

## Testing Patterns

### 1. Testing Configuration Changes

```cpp
TEST_F(MyPluginTest, UpdatesInternalStateOnConfig) {
    // Arrange
    EXPECT_CALL(mock_plugin, OnConfigChanged("Key", "Value"))
        .Times(1);
    
    // Act
    mock_plugin.SimulateConfigChange("Key", "Value");
    
    // Assert
    // (gmock verifies the expectation)
}
```

### 2. Testing Message Processing

```cpp
TEST_F(MyPluginTest, ProcessesIncomingMessages) {
    // Arrange
    int message_count = 0;
    EXPECT_CALL(mock_plugin, OnMessageReceived(_))
        .WillRepeatedly(Invoke([&](IvpMessage*) {
            message_count++;
        }));
    
    // Act
    IvpMessage msg = {};
    mock_plugin.OnMessageReceived(&msg);
    mock_plugin.OnMessageReceived(&msg);
    
    // Assert
    EXPECT_EQ(2, message_count);
}
```

### 3. Testing State Transitions

```cpp
TEST_F(MyPluginTest, HandlesStateTransitions) {
    // Arrange
    std::vector<IvpPluginState> states;
    EXPECT_CALL(mock_plugin, OnStateChange(_))
        .WillRepeatedly(Invoke([&](IvpPluginState state) {
            states.push_back(state);
        }));
    
    // Act
    mock_plugin.SimulateStateChange(IVP_STATUS_RUNNING);
    mock_plugin.SimulateStateChange(IVP_STATUS_PAUSED);
    mock_plugin.SimulateStateChange(IVP_STATUS_STOPPED_DISCONENCTED);
    
    // Assert
    EXPECT_EQ(3, states.size());
}
```

### 4. Testing Error Handling

```cpp
TEST_F(MyPluginTest, HandlesErrors) {
    // Arrange
    EXPECT_CALL(mock_plugin, OnError(IVP_ERR_GENERAL))
        .Times(1);
    
    // Act
    mock_plugin.OnError(IVP_ERR_GENERAL);
    
    // Assert (gmock verifies)
}
```

## Advanced Techniques

### Capturing Arguments

```cpp
IvpPluginState captured_state;
EXPECT_CALL(mock_plugin, OnStateChange(_))
    .WillOnce(Invoke([&captured_state](IvpPluginState state) {
        captured_state = state;
    }));

mock_plugin.SimulateStateChange(IVP_STATUS_RUNNING);
EXPECT_EQ(IVP_STATUS_RUNNING, captured_state);
```

### Sequence Checking

```cpp
using ::testing::InSequence;

InSequence seq;
EXPECT_CALL(mock_plugin, OnConfigChanged("Key1", _)).Times(1);
EXPECT_CALL(mock_plugin, OnStateChange(IVP_STATUS_RUNNING)).Times(1);
EXPECT_CALL(mock_plugin, OnMessageReceived(_)).Times(1);

mock_plugin.SimulateConfigChange("Key1", "Value1");
mock_plugin.SimulateStateChange(IVP_STATUS_RUNNING);
IvpMessage msg = {};
mock_plugin.OnMessageReceived(&msg);
```

### Side Effects

```cpp
std::string captured_value;
EXPECT_CALL(mock_plugin, OnConfigChanged(_, _))
    .WillOnce(Invoke([&](const char *key, const char *value) {
        if (value) captured_value = std::string(value);
    }));

mock_plugin.SimulateConfigChange("Key", "TestValue");
EXPECT_EQ("TestValue", captured_value);
```

## Troubleshooting

### Issue: Undefined Reference to PluginClient Methods

**Solution**: Ensure you're linking against the correct libraries:

```cmake
target_link_libraries(my_test
    plugin_mocks
    tmx-utils  # Contains PluginClient implementation
    ${GTEST_LIBRARIES}
)
```

### Issue: GMock Expectations Not Being Met

**Solution**: Use `WillRepeatedly` or `Times(AnyNumber())` if you're unsure about call counts:

```cpp
// Flexible - accepts any number of calls
EXPECT_CALL(mock_plugin, OnStateChange(_))
    .WillRepeatedly(::testing::Return());
```

### Issue: Compilation Errors with template methods

**Solution**: Ensure you have:

```cpp
#include <gmock/gmock.h>
#include <gtest/gtest.h>
```

And use the correct namespacing:

```cpp
using ::testing::Invoke;
using ::testing::Return;
```

### Issue: Static Initialization Problems

**Solution**: If you get static initialization errors, ensure the test doesn't depend on global state:

```cpp
// Bad - depends on global state
static PluginClient* g_plugin;

// Good - local to each test
std::unique_ptr<MyPlugin> plugin;
```

## Running Tests with Coverage

```bash
# Build with coverage flags
cd build
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make
ctest --verbose

# Generate coverage report (if gcov is installed)
gcov ExamplePluginTest.cpp
```

## Best Practices

1. **One assertion concept per test** - Test one behavior per test function
2. **Use descriptive names** - Test names should describe what is being tested
3. **Follow AAA pattern** - Arrange, Act, Assert
4. **Mock external dependencies** - Don't mock the class under test
5. **Test behavior, not implementation** - Test what the plugin does, not how it does it
6. **Use test fixtures** - Reduce setup/teardown code duplication
7. **Verify call sequences** - Use `InSequence` when order matters
8. **Check return values and side effects** - Verify both what the method returns and what it changed

## References

- [Google Test Documentation](https://github.com/google/googletest/blob/main/docs/primer.md)
- [Google Mock Documentation](https://github.com/google/googletest/blob/main/docs/gmock_for_dummies.md)
- [PluginClient API](/src/tmx/TmxUtils/src/PluginClient.h)
- [V2X Hub Main Documentation](/README.md)

## Contributing New Tests

When adding tests for new plugins:

1. Create a new test file in `tests/unit/plugins/`
2. Add the test executable to `CMakeLists.txt`
3. Follow the existing test patterns
4. Ensure all tests pass: `ctest --verbose`
5. Verify code coverage hasn't decreased

## Questions or Issues?

For questions about the mocking framework or these tests, refer to:
- The embedded documentation in `MockPluginClient.h`
- The example tests in `ExamplePluginTest.cpp`
- The GMock and GTest official documentation

---

**Last Updated**: November 2025  
**Version**: 1.0
