project( JSONMessageLoggerPlugin VERSION 7.6.0 LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 17)

set (TMX_PLUGIN_NAME "JSON Message Logger")

find_package(carma-clock REQUIRED)
BuildTmxPlugin()

target_link_libraries(JSONMessageLoggerPlugin PUBLIC  
    tmxutils 
    ::carma-clock )


####################################################
##################  Testing  #######################
####################################################
# add_library(${PROJECT_NAME}_lib src/TelematicUnit.cpp)
# target_link_libraries(${PROJECT_NAME}_lib PUBLIC  tmxutils)
# enable_testing()
# include_directories(${PROJECT_SOURCE_DIR}/src)
# file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
# add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
# add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

# target_link_libraries(${PROJECT_NAME}_test PUBLIC gtest)