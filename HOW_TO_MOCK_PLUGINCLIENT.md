# Summary: How to Mock PluginClient for V2X Hub Unit Testing

## The Problem

You want to unit test V2X Hub plugins that inherit from `PluginClient`, but:
- `PluginClient::__construct()` calls `ivp_create()` which requires the full IVP system
- Database, configuration, and message routing systems are tightly coupled
- Testing becomes impossible without full infrastructure initialization

## The Solution: Google Mock (GMock)

Use GMock to create a mock implementation that:
1. Inherits from `PluginClient` (or your plugin)
2. Mocks all virtual methods
3. Captures calls for verification
4. Allows behavior control in tests

## Quick Start

### 1. Include GMock Headers
```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "PluginClient.h"
```

### 2. Create Test Class
```cpp
class MyPluginTest : public ::testing::Test {
protected:
    std::unique_ptr<MyPlugin> plugin;
    
    void SetUp() override {
        plugin = std::make_unique<MyPlugin>("TestPlugin");
    }
};
```

### 3. Write Tests
```cpp
TEST_F(MyPluginTest, HandlesConfiguration) {
    // Arrange
    const char *key = "EnableFeature";
    const char *value = "true";
    
    // Act
    plugin->OnConfigChanged(key, value);
    
    // Assert
    std::string retrieved;
    EXPECT_TRUE(plugin->GetConfigValue(key, retrieved));
    EXPECT_EQ("true", retrieved);
}
```

## Virtual Methods to Mock

These are the methods in `PluginClient` that you can mock:

```cpp
virtual int Main();
virtual bool ProcessOptions(const boost::program_options::variables_map &);
virtual void OnConfigChanged(const char *key, const char *value);
virtual void OnError(IvpError err);
virtual void OnMessageReceived(IvpMessage *msg);
virtual void OnStateChange(IvpPluginState state);
```

## The Mock Implementation

The framework provides `MockPluginClient` in `tests/unit/mocks/MockPluginClient.h`:

```cpp
class MockPluginClient : public PluginClient {
public:
    MOCK_METHOD(int, Main, (), (override));
    MOCK_METHOD(void, OnConfigChanged, (const char *key, const char *value), (override));
    MOCK_METHOD(void, OnMessageReceived, (IvpMessage *msg), (override));
    // ... etc for all virtual methods
};
```

## Using the Mock

### Direct Mocking
```cpp
MockPluginClient mock_plugin("TestPlugin");

// Set expectations
EXPECT_CALL(mock_plugin, OnStateChange)
    .Times(1);

// Simulate behavior
mock_plugin.SimulateStateChange(IVP_STATUS_RUNNING);
```

### Testing Your Plugin
```cpp
class MyPluginTest : public ::testing::Test {
    std::unique_ptr<MyPlugin> plugin;
    
    void SetUp() {
        plugin = std::make_unique<MyPlugin>("Test");
    }
};

TEST_F(MyPluginTest, ProcessesMessages) {
    IvpMessage msg = {};
    plugin->OnMessageReceived(&msg);
    EXPECT_EQ(1, plugin->GetMessageCount());
}
```

## Key Testing Patterns

### Test Configuration Changes
```cpp
TEST_F(PluginTest, ConfigurationChanges) {
    plugin->OnConfigChanged("Key", "Value");
    EXPECT_EQ("Value", plugin->GetConfigValue("Key"));
}
```

### Test State Transitions
```cpp
TEST_F(PluginTest, StateTransition) {
    plugin->OnStateChange(IVP_STATUS_RUNNING);
    EXPECT_EQ(IVP_STATUS_RUNNING, plugin->GetCurrentState());
}
```

### Test Message Processing
```cpp
TEST_F(PluginTest, MessageProcessing) {
    IvpMessage msg = {};
    plugin->OnMessageReceived(&msg);
    EXPECT_GT(plugin->GetMessageCount(), 0);
}
```

### Test Error Handling
```cpp
TEST_F(PluginTest, ErrorHandling) {
    plugin->OnError(IVP_ERR_GENERAL);
    EXPECT_EQ(IVP_ERR_GENERAL, plugin->GetLastError());
}
```

## Build Configuration

```cmake
find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(my_plugin_test MyPluginTest.cpp)

target_link_libraries(my_plugin_test
    ${GTEST_LIBRARIES}
    gmock
    Boost::program_options
    tmx-utils
)

add_test(NAME MyPluginTest COMMAND my_plugin_test)
```

## Run Tests

```bash
cd tests/unit/build
cmake ..
make
ctest --verbose
```

## GMock Basics

### EXPECT_CALL - Verify Method Calls
```cpp
// Called exactly once
EXPECT_CALL(plugin, OnStateChange)
    .Times(1);

// Called at least once
EXPECT_CALL(plugin, OnStateChange)
    .Times(::testing::AtLeast(1));

// Called any number of times
EXPECT_CALL(plugin, OnStateChange)
    .Times(::testing::AnyNumber());
```

### Return Values
```cpp
// Return specific value
EXPECT_CALL(plugin, ProcessOptions)
    .WillOnce(::testing::Return(true));

// Return different values on subsequent calls
EXPECT_CALL(plugin, Main)
    .WillOnce(::testing::Return(0))
    .WillRepeatedly(::testing::Return(1));

// Call custom function
EXPECT_CALL(plugin, OnError)
    .WillOnce(::testing::Invoke([](IvpError err) {
        std::cout << "Error: " << err << std::endl;
    }));
```

### Match Arguments
```cpp
// Match specific argument
EXPECT_CALL(plugin, OnConfigChanged("Key", _))
    .Times(1);

// Match with predicate
EXPECT_CALL(plugin, OnStateChange(::testing::Eq(IVP_STATUS_RUNNING)))
    .Times(1);

// Use custom matcher
MATCHER_P(MessageOfType, type, "") {
    return arg != nullptr && strcmp(arg->type, type) == 0;
}

EXPECT_CALL(plugin, OnMessageReceived(MessageOfType("J2735")))
    .Times(1);
```

## Files Provided

### Documentation
- `GMOCK_TESTING_OVERVIEW.md` - This file (overview)
- `docs/GMock_PluginClient_Unit_Testing_Guide.md` - Complete guide
- `tests/unit/README.md` - User guide
- `tests/unit/QUICK_REFERENCE.md` - Cheat sheet
- `tests/unit/TROUBLESHOOTING.md` - Problem solver

### Code
- `tests/unit/mocks/MockPluginClient.h` - Mock implementation
- `tests/unit/mocks/MockPluginClient.cpp` - Mock implementation
- `tests/unit/CMakeLists.txt` - Build configuration
- `tests/unit/ExamplePluginTest.cpp` - Basic examples
- `tests/unit/plugins/RealPluginExampleTest.cpp` - Advanced examples

## Next Steps

1. **Read**: `tests/unit/QUICK_REFERENCE.md` (2 minutes)
2. **Study**: `tests/unit/ExamplePluginTest.cpp` (5 minutes)
3. **Create**: Your first test for a simple plugin (15 minutes)
4. **Expand**: Test more plugins and edge cases

## Key Takeaways

✅ Mock virtual methods using `MOCK_METHOD`  
✅ Test plugin logic without IVP infrastructure  
✅ Verify method calls with `EXPECT_CALL`  
✅ Control behavior with `WillOnce` and `WillRepeatedly`  
✅ Match arguments with matchers or predicates  
✅ Build tests with standard CMake configuration  

## Common Issues

| Issue | Solution |
|-------|----------|
| "undefined reference" | Link against tmx-utils library |
| "gmock not found" | Install libgtest-dev package |
| "PluginClient constructor fails" | This is OK - GMock still works |
| "Test crashes on startup" | Catch exception in constructor |
| "Can't mock method" | Ensure method is virtual |

## Learning Resources

- **Google Test Docs**: https://github.com/google/googletest
- **Full Guide**: `docs/GMock_PluginClient_Unit_Testing_Guide.md`
- **Examples**: `tests/unit/ExamplePluginTest.cpp`
- **Cheat Sheet**: `tests/unit/QUICK_REFERENCE.md`

---

**That's it!** You now understand how to:
1. Create GMock mocks of base classes
2. Test child classes that inherit from PluginClient
3. Verify method calls and control behavior
4. Build and run unit tests

**Start testing your plugins now!** 🚀

For detailed information, see the complete documentation files included in the framework.
