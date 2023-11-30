PROJECT (TelematicBridgePlugin VERSION 7.5.1 LANGUAGES CXX)

set (TMX_PLUGIN_NAME "Telematic Bridge")

BuildTmxPlugin()
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils jsoncpp nats)

####################################################
##################  Testing  #######################
####################################################
add_library(${PROJECT_NAME}_lib src/TelematicUnit.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC  tmxutils jsoncpp nats)
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_test PRIVATE gtest ${PROJECT_NAME}_lib)