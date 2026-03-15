# GMock PluginClient Testing - Visual Quick Guide

## The One-Minute Version

```cpp
// Step 1: Include headers
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Step 2: Create test class
class MyPluginTest : public ::testing::Test {
protected:
    std::unique_ptr<MyPlugin> plugin;
    void SetUp() { plugin = std::make_unique<MyPlugin>("Test"); }
};

// Step 3: Write test
TEST_F(MyPluginTest, TestsFeature) {
    plugin->OnConfigChanged("Key", "Value");
    EXPECT_EQ("Value", plugin->GetConfigValue("Key"));
}

// Step 4: Run
// $ cmake .. && make && ctest --verbose
```

## The Five-Pattern Solution

### Pattern 1: Test Configuration
```cpp
plugin->OnConfigChanged("Key", "Value");
EXPECT_EQ("Value", plugin->GetConfigValue("Key"));
```

### Pattern 2: Test State
```cpp
plugin->OnStateChange(IVP_STATUS_RUNNING);
EXPECT_EQ(IVP_STATUS_RUNNING, plugin->GetCurrentState());
```

### Pattern 3: Test Messages
```cpp
IvpMessage msg = {};
plugin->OnMessageReceived(&msg);
EXPECT_GT(plugin->GetMessageCount(), 0);
```

### Pattern 4: Test Errors
```cpp
plugin->OnError(IVP_ERR_GENERAL);
EXPECT_EQ(IVP_ERR_GENERAL, plugin->GetLastError());
```

### Pattern 5: Mock Behavior
```cpp
using ::testing::Return;
EXPECT_CALL(mock, Method())
    .WillOnce(Return(true));
```

## What to Mock

```
PluginClient (Base Class)
├── virtual int Main()
├── virtual bool ProcessOptions(...)
├── virtual void OnConfigChanged(key, value)
├── virtual void OnError(err)
├── virtual void OnMessageReceived(msg)
└── virtual void OnStateChange(state)
        ↓
        MOCK ALL OF THESE!
```

## Three Ways to Use It

### Way 1: Direct Plugin Test
```cpp
class MyPlugin : public PluginClient { /* ... */ };

TEST(MyPluginTest, TestFeature) {
    MyPlugin plugin("Test");
    plugin->ProcessData();
    EXPECT_TRUE(plugin->IsSuccess());
}
```

### Way 2: Using Mock
```cpp
MockPluginClient mock("Test");

EXPECT_CALL(mock, OnStateChange(_))
    .Times(1);

mock.SimulateStateChange(IVP_STATUS_RUNNING);
```

### Way 3: Test Your Plugin with Mock
```cpp
class YourPlugin : public PluginClient { /* ... */ };

TEST(YourPluginTest, Feature) {
    YourPlugin plugin("Test");
    EXPECT_TRUE(plugin->IsConfigured());
}
```

## Build Steps

```bash
# 1. Create test file (MyPluginTest.cpp)
# 2. Copy CMakeLists.txt from tests/unit/
# 3. Build
mkdir build && cd build
cmake ..
make

# 4. Run
ctest --verbose
```

## CMakeLists.txt Essentials

```cmake
find_package(GTest REQUIRED)
add_executable(my_test MyPluginTest.cpp)
target_link_libraries(my_test ${GTEST_LIBRARIES} gmock tmx-utils)
add_test(NAME MyTest COMMAND my_test)
```

## The Big Picture

```
┌─────────────────────────────────────┐
│      Your Plugin (MyPlugin)         │  
│  inherits from PluginClient         │
└──────────────┬──────────────────────┘
               │
               ├─ Inherits all virtual methods
               │  (OnConfigChanged, OnMessageReceived, etc.)
               │
               └─ You implement/override them
               
┌──────────────────────────────────────┐
│  Test (MyPluginTest)                 │
│  inherits from ::testing::Test       │
└──────────────┬───────────────────────┘
               │
               ├─ Creates MyPlugin instance
               │
               ├─ Calls virtual methods
               │
               └─ Verifies behavior with EXPECT_*

When you can't init PluginClient:
┌──────────────────────────────────────┐
│  MockPluginClient                    │
│  (framework provides)                │
│  inherits from PluginClient          │
└──────────────┬───────────────────────┘
               │
               ├─ Mocks all virtual methods
               │
               ├─ Allows behavior control
               │
               └─ Captures method calls
```

## Cheat Sheet

| Goal | Code |
|------|------|
| Test configuration | `plugin->OnConfigChanged("k", "v");` + `EXPECT_EQ()` |
| Test state | `plugin->OnStateChange(state);` + `EXPECT_EQ()` |
| Test message | `IvpMessage m;` + `plugin->OnMessageReceived(&m);` + `EXPECT_*()` |
| Mock method | `MOCK_METHOD(Return, Name, (Args), (override));` |
| Expect call | `EXPECT_CALL(mock, Method()).Times(1);` |
| Return value | `.WillOnce(::testing::Return(value));` |
| Check args | `EXPECT_CALL(mock, Method("exact_arg", _));` |
| Call function | `.WillOnce(::testing::Invoke(func));` |

## Error Codes to Know

```cpp
IVP_STATUS_OK                  // Plugin initialized
IVP_STATUS_RUNNING             // Plugin running
IVP_STATUS_PAUSED              // Plugin paused
IVP_STATUS_STOPPED_DISCONENCTED // Plugin stopped

IVP_ERR_NONE                   // No error
IVP_ERR_GENERAL                // General error
IVP_ERR_UNINITIALIZED_PLUGIN   // Not initialized
```

## File Locations

```
V2X-Hub/
├── HOW_TO_MOCK_PLUGINCLIENT.md          ← Read first!
├── GMOCK_TESTING_OVERVIEW.md            ← Complete overview
├── GMOCK_TESTING_INDEX.md               ← Documentation index
├── docs/
│   └── GMock_PluginClient_Unit_Testing_Guide.md  ← Full reference
└── tests/unit/
    ├── QUICK_REFERENCE.md               ← Copy-paste templates
    ├── README.md                        ← Usage guide
    ├── TROUBLESHOOTING.md               ← Problem solver
    ├── CMakeLists.txt                   ← Build config (copy this!)
    ├── ExamplePluginTest.cpp            ← Basic examples
    ├── mocks/
    │   ├── MockPluginClient.h           ← Mock implementation
    │   └── MockPluginClient.cpp         ← Mock implementation
    └── plugins/
        └── RealPluginExampleTest.cpp    ← Advanced examples
```

## Common Mistakes

❌ Don't forget `#include <gmock/gmock.h>`  
❌ Don't forget `.override` in mock method  
❌ Don't link against wrong libraries  
❌ Don't put business logic in tests  
❌ Don't test infrastructure (test plugin logic)  

## Success Checklist

✅ Read HOW_TO_MOCK_PLUGINCLIENT.md  
✅ Copy CMakeLists.txt from tests/unit/  
✅ Look at ExamplePluginTest.cpp  
✅ Create MyPluginTest.cpp  
✅ Include gtest and gmock  
✅ Create test class with SetUp()  
✅ Write first TEST_F  
✅ Build and run  
✅ All tests pass!  

## Next: Read These Files

**For Quick Start:**
→ `HOW_TO_MOCK_PLUGINCLIENT.md` (5 min)
→ `tests/unit/QUICK_REFERENCE.md` (10 min)

**For Examples:**
→ `tests/unit/ExamplePluginTest.cpp`
→ `tests/unit/plugins/RealPluginExampleTest.cpp`

**For Deep Dive:**
→ `docs/GMock_PluginClient_Unit_Testing_Guide.md`

**For Problems:**
→ `tests/unit/TROUBLESHOOTING.md`

---

That's it! You're ready to write unit tests for V2X Hub plugins! 🚀

**Remember:** 
- Virtual methods get mocked
- Tests are isolated from infrastructure  
- GMock verifies behavior
- EXPECT_CALL checks calls were made
- Focus on plugin logic, not framework
