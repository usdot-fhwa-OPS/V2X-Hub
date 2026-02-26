# CMakeLists.txt for TmxCtl tests

# Find required packages
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

# Include directories
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/lib)

# Test source files
set(TEST_SOURCES
    test_PluginStatus.cpp
)

# Create test executable
add_executable(tmxctl_test ${TEST_SOURCES})

# Link libraries
target_link_libraries(tmxctl_test
    ${TMXCTL_LIBRARIES}
    ${TMXUTILS_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    pthread
)

# Add test to CTest
add_test(NAME TmxCtlTests COMMAND tmxctl_test)

# Set test properties
set_tests_properties(TmxCtlTests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Enable testing
enable_testing()
