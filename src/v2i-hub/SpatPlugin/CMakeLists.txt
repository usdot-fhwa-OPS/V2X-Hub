project( SpatPlugin VERSION 7.6.0 LANGUAGES CXX )
set(TMX_PLUGIN_NAME "SPAT")
set(CMAKE_CXX_STANDARD 17)
find_package(carma-clock REQUIRED)
BuildTmxPlugin()
target_link_libraries(${PROJECT_NAME} tmxutils ::carma-clock)
add_library(${PROJECT_NAME}_lib src/NTCIP1202.cpp
                                src/SignalControllerConnection.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC tmxutils ::carma-clock )
#############
## Testing ##
#############
enable_testing()
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_include_directories(${BINARY} PUBLIC  src/) 
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest gmock gtest_main)
