PROJECT ( CARMASimulationAdapter VERSION 7.4.0 LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE( carma-clock )
BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils ::carma-clock rdkafka++ jsoncpp)

# ############
# # Testing ##
# ############
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(${PROJECT_NAME}_lib src/CARMASimulationAdapter.cpp src/CARMASimulationConnection.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC tmxutils ::carma-clock rdkafka++ jsoncpp )
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest gmock)