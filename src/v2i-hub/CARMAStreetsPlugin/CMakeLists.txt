PROJECT ( CARMAStreetsPlugin VERSION 7.4.0 LANGUAGES CXX )

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils  rdkafka++ jsoncpp)

#############
## Testing ##
#############
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(${PROJECT_NAME}_lib src/J2735MapToJsonConverter.cpp src/JsonToJ2735SpatConverter.cpp src/J2735ToSRMJsonConverter.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${TMXAPI_LIBRARIES} 
                                        ${ASN_J2735_LIBRARIES} 
                                        ${MYSQL_LIBRARIES} 
                                        ${MYSQLCPPCONN_LIBRARIES}
                                        tmxutils
                                        ${UUID_LIBRARY})
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest jsoncpp)