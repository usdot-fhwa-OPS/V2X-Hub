PROJECT ( SpatPlugin VERSION 7.5.1 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "SPAT")

FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)
FIND_PACKAGE (carma-clock REQUIRED)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ::carma-clock rdkafka++ jsoncpp ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES})

################################
# GTest
################################
enable_testing()

add_library(${PROJECT_NAME}_spat_lib src/NTCIP1202.cpp src/signalController.cpp src/PedestrianDetectionForSPAT.cpp)
target_include_directories( ${PROJECT_NAME}_spat_lib PUBLIC  
                                                    ${PROJECT_SOURCE_DIR}/src
                                                    ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_spat_lib PUBLIC 
                                                tmxutils ::carma-clock ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES})
#############
## Testing ##
#############
set(BINARY ${PROJECT_NAME}_test)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)

set(SOURCES ${TEST_SOURCES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_spat_lib ${TMXAPI_LIBRARIES} 
                                        ${ASN_J2735_LIBRARIES} 
                                        ${MYSQL_LIBRARIES} 
                                        ${MYSQLCPPCONN_LIBRARIES}
                                        tmxutils
                                        ${UUID_LIBRARY}
                                        gtest)