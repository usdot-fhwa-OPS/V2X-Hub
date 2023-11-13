PROJECT(RSUHealthMonitorPlugin VERSION 7.5.1 LANGUAGES CXX)

set(TMX_PLUGIN_NAME "RSU Health Monitor")

find_library(NETSNMPAGENT "netsnmpagent")
find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")
find_library(libasn1c .)

BuildTmxPlugin()

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC tmxutils ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP} ${NETSNMP_LIBRARIES} jsoncpp NemaTode)

#############
## Testing ##
#############
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(${PROJECT_NAME}_lib src/RSUHealthMonitorWorker.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${TMXAPI_LIBRARIES} 
                                        ${ASN_J2735_LIBRARIES} 
                                        ${MYSQL_LIBRARIES} 
                                        ${MYSQLCPPCONN_LIBRARIES}
                                        tmxutils
                                        NemaTode
                                        ${UUID_LIBRARY})
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest)