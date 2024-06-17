PROJECT(MUSTSensorDriverPlugin VERSION 7.6.0 LANGUAGES CXX)

set(TMX_PLUGIN_NAME "Must Sensor Driver Plugin")


BuildTmxPlugin()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC tmxutils )

#############
## Testing ##
#############
enable_testing()
# add_library(${PROJECT_NAME}_lib )
# target_link_libraries(${PROJECT_NAME}_lib PUBLIC 
#                                         tmxutils
                                        
#                                         )
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC 
        # ${PROJECT_NAME}_lib 
        gtest)