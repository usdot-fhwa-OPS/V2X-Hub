project( ImmediateForwardPlugin VERSION 7.10.0 LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 17)

set(TMX_PLUGIN_NAME "Immediate Forward")
find_package(jsoncpp REQUIRED)
find_package(CURL REQUIRED)

BuildTmxPlugin ()

target_include_directories(${PROJECT_NAME} PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} tmxutils jsoncpp )
#############
## Testing ##
#############
enable_testing()
add_library(${PROJECT_NAME}_lib src/ImmediateForwardConfiguration.cpp src/IMFNTCIP1218Worker.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC tmxutils jsoncpp)
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

target_link_libraries(${PROJECT_NAME}_test PUBLIC ${PROJECT_NAME}_lib gtest)