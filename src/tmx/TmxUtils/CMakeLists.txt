project( tmxutils CXX )
file(GLOB_RECURSE HEADERS "src/" "src/*.h*")
file(GLOB_RECURSE SOURCES  "src/" "src/*.c*")
find_library(NETSNMPAGENT "netsnmpagent")
find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")
message(status "sources: ${SOURCES}")
FIND_PACKAGE (carma-clock)
FIND_LIBRARY (UUID_LIBRARY uuid)
ADD_LIBRARY (${PROJECT_NAME} STATIC ${SOURCES})

IF (TMX_LIB_DIR)
    SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${TMX_LIB_DIR}")
ENDIF ()

TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME} SYSTEM PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                            $<BUILD_INTERFACE:${TMXMESSAGES_INCLUDE_DIR}>
                            ${MYSQL_INCLUDE_DIR}
                            ${NETSNMP_INCLUDE_DIRS}
                            ${MYSQLCPPCONN_INCLUDE_DIR})
TARGET_LINK_LIBRARIES (${PROJECT_NAME} PUBLIC 
                       ${TMXAPI_LIBRARIES} 
                       ${MYSQL_LIBRARIES} 
                       ${MYSQLCPPCONN_LIBRARIES}
                       ${UUID_LIBRARY} 
                       ${NETSNMPAGENT} 
                       ${NETSNMPMIBS} 
                       ${NETSNMP} 
                       ${NETSNMP_LIBRARIES} 
                       rdkafka++ 
                       ::carma-clock
                       pthread
                       m
                       rt)
                       
SET (TMXUTILS_LIBRARIES ${PROJECT_NAME} PARENT_SCOPE)

INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${TMX_APPNAME} DESTINATION lib COMPONENT lib${PROJECT_NAME})
INSTALL(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h*")
# INSTALL (EXPORT ${TMX_APPNAME}-plugin DESTINATION share/${TMX_APPNAME}/${TMX_APPNAME}-plugin.cmake)
# EXPORT (TARGETS ${PROJECT_NAME} ${TMXAPI_LIBRARIES} ${ASN_J2735_LIBRARIES} 
#        FILE ${CMAKE_SOURCE_DIR}/${TMX_APPNAME}-plugin.cmake)

################################
#GTest
###############################
enable_testing()

#############
## Testing ##
#############
set(BINARY ${PROJECT_NAME}_test)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)


add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME} rdkafka++ gmock ${TMXAPI_LIBRARIES} ${ASN_J2735_LIBRARIES} ${UUID_LIBRARY} ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP} ${NETSNMP_LIBRARIES}  gtest)