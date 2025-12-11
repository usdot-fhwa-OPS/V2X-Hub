# GMock PluginClient Testing - Quick Reference

## Quick Start Template

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "YourPlugin.h"

using ::testing::_;

class YourPluginTest : public ::testing::Test {
protected:
    std::unique_ptr<YourPlugin> plugin;
    
    void SetUp() override {
        plugin = std::make_unique<YourPlugin>("TestPlugin");
    }
};

TEST_F(YourPluginTest, TestSomething) {
    // Arrange
    // Act
    // Assert
}
```

## Key Concepts

### 1. Virtual Methods to Mock

```cpp
class PluginClient {
    virtual int Main();
    virtual bool ProcessOptions(...);
    virtual void OnConfigChanged(const char *key, const char *value);
    virtual void OnError(IvpError err);
    virtual void OnMessageReceived(IvpMessage *msg);
    virtual void OnStateChange(IvpPluginState state);
};
```

### 2. Non-Virtual Methods (Cannot Mock Directly)

- `BroadcastMessage()` - Override or use test hooks
- `GetConfigValue()` - Call directly, returns real value
- `SetStatus()` - Calls real IVP system

### 3. Template Methods

These are templates that call virtual methods internally:

```cpp
template <typename MsgType>
void BroadcastMessage(MsgType& message, ...);

template <typename T>
bool GetConfigValue(const std::string &key, T &value, ...);
```

## Common Test Patterns

### Test Configuration Changes

```cpp
TEST_F(PluginTest, HandlesConfig) {
    // Simulate a config change
    plugin->OnConfigChanged("Key", "Value");
    
    // Verify internal state
    std::string val;
    plugin->GetConfigValue("Key", val);
    EXPECT_EQ("Value", val);
}
```

### Test Message Processing

```cpp
TEST_F(PluginTest, ProcessesMessages) {
    IvpMessage msg = {};
    msg.type = const_cast<char*>("TestType");
    
    plugin->OnMessageReceived(&msg);
    
    EXPECT_EQ(1, plugin->GetMessageCount());
}
```

### Test State Transitions

```cpp
TEST_F(PluginTest, ChangesState) {
    plugin->OnStateChange(IVP_STATUS_RUNNING);
    EXPECT_EQ(IVP_STATUS_RUNNING, plugin->GetCurrentState());
}
```

### Test Error Handling

```cpp
TEST_F(PluginTest, HandlesErrors) {
    plugin->OnError(IVP_ERR_GENERAL);
    EXPECT_EQ(IVP_ERR_GENERAL, plugin->GetLastError());
}
```

## GMock Expectations

### Basic Expectations

```cpp
EXPECT_CALL(mock, Method)
    .Times(1);                          // Called exactly once
    .Times(AtLeast(1));                 // Called 1+ times
    .Times(AnyNumber());                // Called any number of times
    .Times(Between(2, 4));              // Called 2-4 times
```

### Return Values

```cpp
EXPECT_CALL(mock, Method)
    .WillOnce(Return(true));            // Return true on first call
    .WillRepeatedly(Return(false));     // Return false on subsequent calls
    .WillOnce(Invoke(function));        // Call custom function
```

### Matching Arguments

```cpp
EXPECT_CALL(mock, OnConfigChanged("Key", _))  // Exact key, any value
    .Times(1);

EXPECT_CALL(mock, OnStateChange(Eq(IVP_STATUS_RUNNING)))
    .Times(1);

EXPECT_CALL(mock, OnMessageReceived(IvpMessageOfType("J2735", "BSM")))
    .Times(1);
```

### Capturing Arguments

```cpp
IvpPluginState captured_state;
EXPECT_CALL(mock, OnStateChange(_))
    .WillOnce(Invoke([&captured_state](IvpPluginState state) {
        captured_state = state;
    }));

mock.OnStateChange(IVP_STATUS_RUNNING);
EXPECT_EQ(IVP_STATUS_RUNNING, captured_state);
```

## Custom Matchers

```cpp
// Match message type
MATCHER_P(IvpMessageOfType, type, "") {
    return arg && arg->type && strcmp(arg->type, type) == 0;
}

// Usage
EXPECT_CALL(mock, OnMessageReceived(IvpMessageOfType("J2735")));
```

## Build Configuration

### CMakeLists.txt Minimal Template

```cmake
find_package(GTest REQUIRED)
find_package(Boost REQUIRED)

add_executable(my_test MyTest.cpp)

target_link_libraries(my_test
    ${GTEST_LIBRARIES}
    gmock
    Boost::program_options
    tmx-utils
)

add_test(NAME MyTest COMMAND my_test)
```

### Build and Run

```bash
mkdir build && cd build
cmake ..
make
ctest --verbose
```

## Common Issues & Solutions

| Problem | Solution |
|---------|----------|
| "undefined reference to PluginClient" | Link against tmx-utils library |
| "gmock not found" | Install gmock or ensure GTest package includes it |
| "IVP system call fails" | Don't initialize PluginClient in tests, mock it |
| "Static initialization issues" | Use local test fixtures, not global state |
| "Can't mock non-virtual method" | Provide test hook or override method |

## Must-Have Includes

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "PluginClient.h"

// Namespacing
using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::AnyNumber;
using ::testing::AtLeast;
```

## Test Execution

```bash
# Run all tests
ctest --verbose

# Run specific test
./MyPluginTest

# Run with output on failure
ctest --verbose --output-on-failure

# Run with timeout per test
ctest --timeout 30 --verbose
```

## Advanced: Sequencing

```cpp
using ::testing::InSequence;

InSequence seq;  // Enforce call order
EXPECT_CALL(mock, OnConfigChanged(_, _)).Times(1);
EXPECT_CALL(mock, OnStateChange(_)).Times(1);
EXPECT_CALL(mock, OnMessageReceived(_)).Times(1);

// Must call in exact order or test fails
```

## Advanced: Callbacks

```cpp
std::vector<std::string> values;
EXPECT_CALL(mock, OnConfigChanged(_, _))
    .WillRepeatedly(Invoke([&values](const char* k, const char* v) {
        values.push_back(std::string(v));
    }));

mock.OnConfigChanged("key", "value1");
mock.OnConfigChanged("key", "value2");

EXPECT_EQ(2, values.size());
```

## Testing Checklist

- [ ] Test configuration loading
- [ ] Test state transitions
- [ ] Test message handling
- [ ] Test error conditions
- [ ] Test data processing
- [ ] Test message broadcasting
- [ ] Test lifecycle (init → running → shutdown)
- [ ] Test concurrent operations (if applicable)
- [ ] Test edge cases (null, empty, invalid values)
- [ ] Test performance (if critical)

## File Structure

```
tests/unit/
├── mocks/
│   ├── MockPluginClient.h
│   └── MockPluginClient.cpp
├── plugins/
│   ├── YourPluginTest.cpp
│   └── RealPluginExampleTest.cpp
├── CMakeLists.txt
└── README.md
```

## Links

- [Full Guide](./GMock_PluginClient_Unit_Testing_Guide.md)
- [Example Tests](./ExamplePluginTest.cpp)
- [Real Plugin Example](./plugins/RealPluginExampleTest.cpp)
- [GMock Documentation](https://github.com/google/googletest/tree/main/googlemock)
- [GTest Primer](https://github.com/google/googletest/blob/main/docs/primer.md)

---

**Version**: 1.0 | **Last Updated**: November 2025
