# V2X Hub Plugin Unit Testing with GMock - Complete Overview

## What You've Received

A complete, production-ready framework for unit testing V2X Hub plugins using Google Mock (GMock). This includes:

### 📚 Documentation Files

1. **GMock_PluginClient_Unit_Testing_Guide.md** (Main Reference)
   - Comprehensive guide explaining GMock concepts
   - Step-by-step implementation instructions
   - Best practices and patterns
   - Troubleshooting tips

2. **README.md** (User Guide)
   - Quick start instructions
   - Directory structure
   - Usage patterns and examples
   - Common troubleshooting

3. **QUICK_REFERENCE.md** (Cheat Sheet)
   - Template code snippets
   - Common patterns
   - Quick command reference
   - Issue quick fixes

4. **TROUBLESHOOTING.md** (Problem Solver)
   - Build issues and solutions
   - Runtime problems
   - GMock-specific issues
   - Debugging techniques

### 🔧 Implementation Files

1. **mocks/MockPluginClient.h** (Mock Base Class)
   - Complete mock of PluginClient
   - Virtual method mocks
   - Message capture capability
   - Helper methods for testing

2. **mocks/MockPluginClient.cpp** (Implementation)
   - Mock constructor/destructor
   - Integration with PluginClient

3. **CMakeLists.txt** (Build Configuration)
   - Proper linking configuration
   - Test setup
   - Coverage options
   - Compiler warnings

### 📝 Example Files

1. **ExamplePluginTest.cpp** (Basic Examples)
   - Simple plugin testing examples
   - Configuration testing
   - State change handling
   - Message processing
   - GMock expectation examples

2. **plugins/RealPluginExampleTest.cpp** (Advanced Examples)
   - Realistic plugin implementation
   - Complete lifecycle testing
   - Integration test patterns
   - Stress testing examples
   - Error handling

## How It Works

### The Problem
Testing V2X Hub plugins directly is difficult because:
- PluginClient constructor calls `ivp_create()` which requires full IVP infrastructure
- Database connections are initialized
- Static state management makes testing complicated
- Message routing system is tightly coupled

### The Solution
GMock allows you to:
1. **Mock virtual methods** - Override plugin behavior for testing
2. **Inject dependencies** - Replace real implementations with test doubles
3. **Capture calls** - Verify methods were called with expected arguments
4. **Control behavior** - Make methods return specific values in tests
5. **Isolate logic** - Test plugin code without infrastructure

### Architecture

```
Your Plugin (MyPlugin)
        ↓
    Inherits from
        ↓
   PluginClient
        ↓
   Inherits from
        ↓
   Runnable
```

In tests:
```
YourPluginTest
        ↓
   Inherits from
        ↓
   ::testing::Test
        ↓
   Creates instance of
        ↓
   Your Plugin
        ↓
   Which inherits from
        ↓
   MockPluginClient (in mock tests)
   or PluginClient (integration tests)
```

## Getting Started

### Step 1: Set Up Build Configuration

Copy the provided CMakeLists.txt to your tests/unit directory and ensure:
- GTest is installed
- Boost is available
- TMX utilities are linked

```bash
# Verify dependencies
find_package(GTest)  # Should succeed
find_package(Boost)  # Should succeed
```

### Step 2: Use MockPluginClient

For testing plugin behavior in isolation:

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "YourPlugin.h"

class YourPluginTest : public ::testing::Test {
protected:
    std::unique_ptr<YourPlugin> plugin;
    
    void SetUp() override {
        plugin = std::make_unique<YourPlugin>("TestPlugin");
    }
};

TEST_F(YourPluginTest, TestsYourFeature) {
    // Arrange
    plugin->OnConfigChanged("Key", "Value");
    
    // Act
    plugin->ProcessData();
    
    // Assert
    EXPECT_EQ("expected", plugin->GetResult());
}
```

### Step 3: Run Tests

```bash
mkdir build
cd build
cmake ..
make
ctest --verbose
```

## Key Concepts

### Virtual Methods You Can Mock

```cpp
MOCK_METHOD(int, Main, (), (override));
MOCK_METHOD(bool, ProcessOptions, (...), (override));
MOCK_METHOD(void, OnConfigChanged, (const char*, const char*), (override));
MOCK_METHOD(void, OnError, (IvpError), (override));
MOCK_METHOD(void, OnMessageReceived, (IvpMessage*), (override));
MOCK_METHOD(void, OnStateChange, (IvpPluginState), (override));
```

### Example: Testing Configuration

```cpp
TEST_F(PluginTest, HandlesConfiguration) {
    // Simulate config change
    plugin->OnConfigChanged("Enabled", "true");
    
    // Verify it was processed
    std::string value;
    EXPECT_TRUE(plugin->GetConfigValue("Enabled", value));
    EXPECT_EQ("true", value);
}
```

### Example: Mocking Behavior

```cpp
using ::testing::Return;
using ::testing::Invoke;

MockPluginClient mock_plugin("Test");

// Make method return a specific value
EXPECT_CALL(mock_plugin, Main())
    .WillOnce(Return(0));

// Execute method with side effects
EXPECT_CALL(mock_plugin, OnStateChange(_))
    .WillOnce(Invoke([](IvpPluginState state) {
        std::cout << "State changed to: " << state << std::endl;
    }));
```

## File Organization

```
V2X-Hub/
├── tests/
│   └── unit/
│       ├── CMakeLists.txt                    # Build config
│       ├── README.md                         # User guide
│       ├── QUICK_REFERENCE.md                # Cheat sheet
│       ├── TROUBLESHOOTING.md                # Problem solver
│       ├── ExamplePluginTest.cpp             # Basic examples
│       ├── mocks/
│       │   ├── MockPluginClient.h            # Mock class
│       │   └── MockPluginClient.cpp          # Implementation
│       └── plugins/
│           ├── RealPluginExampleTest.cpp     # Advanced examples
│           ├── RtcmPluginTest.cpp            # (To create)
│           ├── CommandPluginTest.cpp         # (To create)
│           └── ...other plugin tests...
│
├── docs/
│   └── GMock_PluginClient_Unit_Testing_Guide.md  # Full guide
│
└── src/
    ├── tmx/
    │   └── TmxUtils/src/
    │       ├── PluginClient.h
    │       └── PluginClient.cpp
    └── v2i-hub/
        ├── YourPlugin/src/YourPlugin.h
        └── ...plugins...
```

## Testing Patterns

### Configuration Testing
```cpp
plugin->OnConfigChanged("Key", "Value");
EXPECT_EQ("Value", plugin->GetConfigValue("Key"));
```

### State Management
```cpp
plugin->OnStateChange(IVP_STATUS_RUNNING);
EXPECT_EQ(IVP_STATUS_RUNNING, plugin->GetCurrentState());
```

### Message Processing
```cpp
IvpMessage msg{};
plugin->OnMessageReceived(&msg);
EXPECT_EQ(1, plugin->GetMessageCount());
```

### Error Handling
```cpp
plugin->OnError(IVP_ERR_GENERAL);
EXPECT_EQ(IVP_ERR_GENERAL, plugin->GetLastError());
```

## Benefits

✅ **Isolated Testing** - Test plugin logic without infrastructure
✅ **Fast Execution** - No database, no IVP system initialization
✅ **Deterministic** - No flaky tests from timing issues
✅ **Easy Debugging** - Clear error messages, simple execution flow
✅ **Code Coverage** - Can achieve >80% coverage with proper tests
✅ **Regression Prevention** - Catch breaking changes early
✅ **Documentation** - Tests serve as usage examples
✅ **Refactoring Confidence** - Know when changes break functionality

## Common Use Cases

### 1. Testing Business Logic
```cpp
// Test data processing without infrastructure
TEST_F(ProcessorPluginTest, ProcessesDataCorrectly) {
    plugin->ProcessData(testData);
    EXPECT_EQ(expectedResult, plugin->GetResult());
}
```

### 2. Testing Configuration Handling
```cpp
// Verify configuration is correctly applied
TEST_F(ConfigPluginTest, AppliesConfiguration) {
    plugin->OnConfigChanged("Setting", "Value");
    EXPECT_TRUE(plugin->IsConfigured());
}
```

### 3. Testing Message Flow
```cpp
// Verify correct message handling
TEST_F(MessagePluginTest, HandlesMessages) {
    IvpMessage msg = CreateMessage("Type", "SubType");
    plugin->OnMessageReceived(&msg);
    EXPECT_EQ(1, plugin->GetProcessedMessageCount());
}
```

### 4. Testing State Machines
```cpp
// Verify state transitions
TEST_F(StatefulPluginTest, TransitionsCorrectly) {
    plugin->OnStateChange(RUNNING);
    EXPECT_EQ(RUNNING, plugin->GetState());
}
```

## Next Steps

1. **Read the Documentation**
   - Start with QUICK_REFERENCE.md for syntax
   - Read full GMock_PluginClient_Unit_Testing_Guide.md for depth
   - Refer to README.md for workflow

2. **Examine the Examples**
   - Review ExamplePluginTest.cpp for basic patterns
   - Study RealPluginExampleTest.cpp for advanced patterns
   - Understand MockPluginClient.h internals

3. **Create Your First Test**
   - Pick a simple plugin to test
   - Follow the template in QUICK_REFERENCE.md
   - Gradually add more test cases

4. **Expand Test Coverage**
   - Create RtcmPluginTest.cpp
   - Create CommandPluginTest.cpp
   - Test other critical plugins

5. **Integrate with CI/CD**
   - Add test execution to build pipeline
   - Set coverage thresholds
   - Enable test reporting

## Important Notes

### What This Framework Does NOT Do

- ❌ Does not test the IVP plugin system itself
- ❌ Does not test database operations
- ❌ Does not test inter-plugin communication at system level
- ❌ Does not test real message routing

### What This Framework DOES Do

- ✅ Tests plugin business logic
- ✅ Tests configuration handling
- ✅ Tests state management
- ✅ Tests message processing logic
- ✅ Tests error handling
- ✅ Tests plugin lifecycle

### Limitations

1. **Cannot test IVP integration** - The IVP system requires full initialization
2. **Cannot test database operations** - Database access is real or requires separate mocking
3. **Cannot test real message routing** - Broadcasting is captured, not routed

### Solutions

For integration testing, use:
- System tests with full V2X Hub deployment
- Docker-based test environment
- Separate integration test suite

For unit testing, use:
- This GMock framework
- Mock external dependencies
- Focus on plugin business logic

## Troubleshooting Quick Links

- **Build errors?** → See TROUBLESHOOTING.md → "Build Issues"
- **Test crashes?** → See TROUBLESHOOTING.md → "Runtime Issues"
- **Mock not working?** → See TROUBLESHOOTING.md → "Mock-Specific Issues"
- **Don't know how?** → See QUICK_REFERENCE.md → "Quick Start Template"
- **Need depth?** → See GMock_PluginClient_Unit_Testing_Guide.md

## Support Resources

📖 **Documentation Files**
- GMock_PluginClient_Unit_Testing_Guide.md - Comprehensive reference
- tests/unit/README.md - Usage guide
- tests/unit/QUICK_REFERENCE.md - Quick syntax reference
- tests/unit/TROUBLESHOOTING.md - Problem solving

📝 **Example Code**
- ExamplePluginTest.cpp - Basic examples
- RealPluginExampleTest.cpp - Advanced examples
- MockPluginClient.h - Complete mock implementation

🔗 **External Resources**
- [Google Test Primer](https://github.com/google/googletest/blob/main/docs/primer.md)
- [Google Mock for Dummies](https://github.com/google/googletest/blob/main/docs/gmock_for_dummies.md)
- [Advanced GMock Guide](https://github.com/google/googletest/blob/main/docs/gmock_cheat_sheet.md)

## Summary

You now have a complete, professional unit testing framework for V2X Hub plugins. The framework includes:

- ✅ Comprehensive documentation (4 guides + examples)
- ✅ Complete mock implementation (MockPluginClient)
- ✅ Build configuration (CMakeLists.txt)
- ✅ Working examples (basic and advanced)
- ✅ Troubleshooting guide
- ✅ Quick reference

**Next action**: Read `tests/unit/QUICK_REFERENCE.md` to get started immediately, then refer to `GMock_PluginClient_Unit_Testing_Guide.md` for deeper understanding.

---

**Framework Version**: 1.0  
**Last Updated**: November 2025  
**Maintained by**: V2X Hub Development Team  
**License**: Same as V2X Hub (Apache 2.0)
