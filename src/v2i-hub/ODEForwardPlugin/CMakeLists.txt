PROJECT ( ODEForwardPlugin VERSION 7.11.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "ODEForwardPlugin")

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils rdkafka++)

#############
## Testing ##
#############
enable_testing()
add_library(${PROJECT_NAME}_lib 
    src/CommunicationModeHelper.cpp 
    src/UDPMessageForwarder.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC tmxutils)
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${PROJECT_SOURCE_DIR}/src)

set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest gtest_main)