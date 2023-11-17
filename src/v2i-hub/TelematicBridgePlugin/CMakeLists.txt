PROJECT (TelematicBridgePlugin VERSION 7.5.1 LANGUAGES CXX)

set (TMX_PLUGIN_NAME "Telematic Bridge")

find_package(RapidJSON REQUIRED)

BuildTmxPlugin()
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils jsoncpp)

####################################################
##################  Testing  #######################
####################################################
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(${PROJECT_NAME}_lib src/TelematicBridgeMsgWorker.cpp)
target_link_libraries(${PROJECT_NAME}_lib  PUBLIC tmxutils jsoncpp)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME}_lib gtest)