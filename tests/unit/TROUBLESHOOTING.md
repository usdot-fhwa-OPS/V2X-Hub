# GMock PluginClient Testing - Troubleshooting Guide

## Build Issues

### Issue: "fatal error: gmock/gmock.h: No such file or directory"

**Cause**: Google Mock headers not found

**Solutions**:
1. Install Google Test package:
   ```bash
   sudo apt-get install libgtest-dev cmake
   cd /usr/src/gtest && sudo cmake . && sudo make
   sudo cp lib/libgmock* /usr/lib/
   sudo cp include/gmock /usr/include/
   ```

2. Or link to local GTest build:
   ```cmake
   include_directories(/path/to/googletest/googlemock/include)
   link_directories(/path/to/googletest/build/lib)
   ```

### Issue: "undefined reference to PluginClient::..."

**Cause**: Not linking against PluginClient library

**Solution**:
```cmake
# Add to CMakeLists.txt
target_link_libraries(my_test
    plugin_mocks
    tmx-utils          # Contains PluginClient implementation
    ${GTEST_LIBRARIES}
    gmock
)
```

### Issue: "undefined reference to `tmx::utils::DbConnection..."

**Cause**: Database-related PluginClient functions not properly mocked

**Solution**: Create a minimal stub:
```cpp
// Add to test file
namespace tmx {
namespace utils {
// Provide minimal stub if needed
} // namespace utils
} // namespace tmx
```

### Issue: Multiple definition errors

**Cause**: MockPluginClient.cpp compiled multiple times

**Solution**: Ensure CMakeLists.txt creates library once:
```cmake
add_library(plugin_mocks STATIC
    mocks/MockPluginClient.cpp
)

add_executable(my_test my_test.cpp)
target_link_libraries(my_test plugin_mocks)  # Link to library, not .cpp
```

### Issue: "could not find boost/program_options.hpp"

**Cause**: Boost not installed or not found

**Solution**:
```bash
# Install Boost
sudo apt-get install libboost-all-dev

# Or specify in CMakeLists.txt
find_package(Boost 1.70 REQUIRED COMPONENTS program_options filesystem)
target_link_libraries(my_test Boost::program_options Boost::filesystem)
```

## Linker Issues

### Issue: Linker timeout or hangs

**Cause**: Circular dependencies or static initialization issues

**Solutions**:
1. Reduce dependency scope - mock external dependencies
2. Use `extern "C"` for C linkage if mixing C/C++
3. Check for circular includes in header files

### Issue: "relocation R_X86_64_32 against..."

**Cause**: Position Independent Code (PIE) issues

**Solution**: Compile with -fPIC:
```cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
```

## Runtime Issues

### Issue: Test crashes at startup

**Cause**: PluginClient constructor calls ivp_create() which fails

**Solutions**:

1. **Option 1**: Mock the problematic initialization
   ```cpp
   // In test file
   TEST(PluginTest, MyTest) {
       // Avoid directly constructing PluginClient
       // Use a wrapper class instead
   }
   ```

2. **Option 2**: Create a wrapper that bypasses IVP
   ```cpp
   class TestablePlugin : public YourPlugin {
   public:
       // Custom constructor for testing
       TestablePlugin() {
           // Initialize only test-safe members
       }
   };
   ```

3. **Option 3**: Mock IVP functions
   ```cpp
   #include <gmock/gmock.h>
   
   // Mock the IVP C functions
   extern "C" {
       IvpPlugin* __wrap_ivp_create(IvpPluginInformation info) {
           // Return mock object
           return nullptr;
       }
   }
   ```

### Issue: "Segmentation fault" during test

**Cause**: Null pointer or invalid memory access

**Debug Steps**:
1. Run with GDB:
   ```bash
   gdb ./MyPluginTest
   run
   bt  # Backtrace
   ```

2. Use valgrind:
   ```bash
   valgrind --leak-check=full ./MyPluginTest
   ```

3. Add debug output:
   ```cpp
   TEST_F(PluginTest, MyTest) {
       std::cerr << "Starting test\n" << std::flush;
       plugin->OnConfigChanged("Key", "Value");
       std::cerr << "Config changed\n" << std::flush;
   }
   ```

### Issue: Tests pass locally but fail in CI

**Causes**: Environment differences

**Solutions**:
1. Ensure consistent library versions in CI environment
2. Check for hardcoded paths - use relative paths
3. Verify Boost version matches
4. Check for platform-specific issues (32-bit vs 64-bit)

## Mock-Specific Issues

### Issue: EXPECT_CALL not being satisfied

**Debug**:
```cpp
// Check if method is actually being called
EXPECT_CALL(mock, OnStateChange(_))
    .Times(testing::AtLeast(1))  // Flexible expectation
    .WillRepeatedly(testing::Invoke([](IvpPluginState s) {
        std::cout << "OnStateChange called with: " << s << "\n";
    }));

plugin->OnStateChange(IVP_STATUS_RUNNING);
```

**Common Causes**:
- Wrong method signature
- Non-virtual method (can't mock)
- Method not being called by test
- Wrong argument types

### Issue: "Uninteresting mock function call"

**Cause**: Method called but not explicitly expected

**Solution**: Suppress the warning or add expectation:
```cpp
// Option 1: Suppress warning
EXPECT_CALL(mock, OnStateChange(_))
    .WillRepeatedly(testing::Return());

// Option 2: Add specific expectation
EXPECT_CALL(mock, OnStateChange(IVP_STATUS_RUNNING))
    .Times(1);
```

### Issue: Can't mock method from parent class

**Cause**: Method not virtual

**Solution**: Doesn't exist in current PluginClient - all handlers are virtual

If you have a similar issue:
- Check method signature in header
- Ensure `virtual` keyword is present
- Check inheritance is public

### Issue: Matcher not matching expected argument

**Debug**:
```cpp
// Use testing::Gt, testing::Lt, testing::NotNull() matchers
EXPECT_CALL(mock, OnConfigChanged(_, testing::NotNull()))
    .Times(1);

// Use explicit matcher function
MATCHER_P(StringContains, substr, "") {
    return std::string(arg).find(substr) != std::string::npos;
}

EXPECT_CALL(mock, OnConfigChanged("Key", StringContains("value")))
    .Times(1);
```

## Test Execution Issues

### Issue: "Test exceeded timeout"

**Cause**: Test or plugin code hanging

**Solution**: 
1. Add timeout configuration:
   ```cmake
   set_tests_properties(my_test PROPERTIES TIMEOUT 30)
   ```

2. Check for infinite loops:
   ```cpp
   // Use a counter with max limit
   int count = 0;
   while (condition && count < 1000) {
       count++;
   }
   if (count >= 1000) {
       FAIL() << "Infinite loop detected";
   }
   ```

3. Check for blocked threads:
   ```cpp
   // Avoid blocking operations in tests
   // Mock them instead
   ```

### Issue: "Memory leak detected"

**Cause**: Not properly cleaning up resources

**Solution**:
```cpp
class PluginTest : public ::testing::Test {
protected:
    void TearDown() override {
        plugin.reset();  // Explicit cleanup
    }
    
    std::unique_ptr<YourPlugin> plugin;
};
```

### Issue: Tests fail intermittently (flaky tests)

**Causes**: Race conditions, timing issues, global state

**Solutions**:
1. Use synchronization:
   ```cpp
   std::mutex mtx;
   std::unique_lock<std::mutex> lock(mtx);
   condition.wait(lock);
   ```

2. Avoid global state:
   ```cpp
   // Bad
   static PluginClient* g_plugin;
   
   // Good
   std::unique_ptr<PluginClient> plugin;
   ```

3. Reset state between tests:
   ```cpp
   void TearDown() override {
       plugin.reset();
       config_map.clear();
   }
   ```

## Assertion Issues

### Issue: EXPECT_EQ shows confusing diff

**Solution**: Use more specific assertions:
```cpp
// Instead of
EXPECT_EQ(plugin->GetStatus(), "RUNNING");

// Use
EXPECT_THAT(plugin->GetStatus(), testing::Eq("RUNNING"));
std::string expected = "RUNNING";
EXPECT_EQ(expected, plugin->GetStatus());
```

### Issue: Comparison of complex types fails

**Solution**: Implement custom matchers:
```cpp
MATCHER_P(IvpMessageEquals, expected, "") {
    return arg->type == expected->type &&
           arg->subtype == expected->subtype;
}

EXPECT_CALL(mock, OnMessageReceived(IvpMessageEquals(&expected_msg)));
```

## Debugging Tips

### Enable verbose test output

```bash
# See all test details
./MyTest --gtest_verbose=1
./MyTest --gtest_print_time=1
```

### Print GMock call details

```cpp
// In test
EXPECT_CALL(mock, OnConfigChanged(_, _))
    .Times(1)
    .WillOnce([](const char* k, const char* v) {
        std::cerr << "Config: " << k << " = " << v << std::endl;
    });
```

### Use GTest's event listener

```cpp
class DebugListener : public ::testing::EmptyTestEventListener {
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        std::cerr << "Starting: " << test_info.name() << std::endl;
    }
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::TestEventListeners& listeners = 
        ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new DebugListener);
    return RUN_ALL_TESTS();
}
```

## Performance Issues

### Issue: Tests run slowly

**Solutions**:
1. Profile the tests:
   ```bash
   time ./MyTest
   ```

2. Reduce expensive operations:
   ```cpp
   // Avoid in every test
   std::this_thread::sleep_for(std::chrono::seconds(1));
   
   // Mock instead
   EXPECT_CALL(mock, ExpensiveOperation())
       .WillOnce(testing::Return());
   ```

3. Use test fixtures efficiently:
   ```cpp
   // SetUpTestSuite for one-time initialization
   static void SetUpTestSuite() {
       expensive_setup();
   }
   ```

## Getting Help

1. **Check the Examples**: See `ExamplePluginTest.cpp` and `RealPluginExampleTest.cpp`
2. **Review Documentation**: See `GMock_PluginClient_Unit_Testing_Guide.md`
3. **Enable GTest Help**: 
   ```bash
   ./MyTest --help
   ```
4. **GTest Official Docs**: https://github.com/google/googletest
5. **GMock Official Docs**: https://github.com/google/googletest/tree/main/googlemock

## Quick Checklist

When tests fail:

- [ ] Verify linking against correct libraries
- [ ] Check CMakeLists.txt dependencies
- [ ] Ensure gmock/gtest are installed
- [ ] Review method signatures in mock
- [ ] Verify method is virtual in base class
- [ ] Check namespace declarations
- [ ] Inspect compiler warnings
- [ ] Run with verbose flags
- [ ] Check for global state issues
- [ ] Verify test environment (CI vs local)

---

**Version**: 1.0 | **Last Updated**: November 2025 | **Difficulty**: Intermediate
