PROJECT( tmxcore CXX )

FILE (GLOB_RECURSE SOURCES "src/*.c*")

ADD_EXECUTABLE (${PROJECT_NAME} ${SOURCES} )
IF (TMX_BIN_DIR)
    SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TMX_BIN_DIR}")
ENDIF ()

TARGET_INCLUDE_DIRECTORIES( ${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                            ${MYSQL_INCLUDE_DIRS} ${MYSQLCPPCONN_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} PUBLIC ${TMXAPI_LIBRARIES} ${TMXUTILS_LIBRARIES} )
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} PUBLIC ${MYSQL_LIBRARIES} ${MYSQLCPPCONN_LIBRARIES} )
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} PUBLIC pthread m rt )

INSTALL (TARGETS ${PROJECT_NAME} EXPORT ${TMX_APPNAME}
         DESTINATION bin COMPONENT ${PROJECT_NAME})

################################
#GTest
###############################
enable_testing()

#############
## Testing ##
#############
if(BUILD_TESTING)
    set(BINARY ${PROJECT_NAME}_test)

    file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)

    add_executable(${BINARY} ${TEST_SOURCES})

    add_test(NAME ${BINARY} COMMAND ${BINARY})

    target_link_libraries(${BINARY} PUBLIC
        ${TMXAPI_LIBRARIES}
        ${TMXUTILS_LIBRARIES}
        ${MYSQL_LIBRARIES}
        ${MYSQLCPPCONN_LIBRARIES}
        gtest
        gtest_main
        gmock
        pthread
        m
        rt
    )

    target_include_directories(${BINARY} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${MYSQL_INCLUDE_DIRS}
        ${MYSQLCPPCONN_INCLUDE_DIRS}
    )
endif()
