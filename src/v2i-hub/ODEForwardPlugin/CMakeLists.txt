PROJECT ( ODEForwardPlugin VERSION 7.6.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "ODEForwardPlugin")

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils rdkafka++)

#############
## Testing ##
#############
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE LIB_SOURCES LIST_DIRECTORIES false  src/*.cpp)
add_library(${PROJECT_NAME}_lib ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME}_lib PUBLIC tmxutils)
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest)