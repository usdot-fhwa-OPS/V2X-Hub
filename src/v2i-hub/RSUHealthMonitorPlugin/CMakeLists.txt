PROJECT(RSUHealthMonitorPlugin VERSION 7.11.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TMX_PLUGIN_NAME "RSU Health Monitor")

BuildTmxPlugin()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC tmxutils jsoncpp NemaTode)

#############
## Testing ##
#############
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(${PROJECT_NAME}_lib src/RSUHealthMonitorWorker.cpp src/RSUConfigurationList)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC 
                                        tmxutils
                                        NemaTode
                                        jsoncpp)
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest gtest_main)