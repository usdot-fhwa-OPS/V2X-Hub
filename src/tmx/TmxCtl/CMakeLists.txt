PROJECT ( tmxctl CXX )

# Find required packages for testing
find_package(Boost REQUIRED COMPONENTS program_options regex system)

SET (LIBRARY_NAME ${PROJECT_NAME}Static)

FILE (GLOB_RECURSE LIB_SOURCES "src/lib/*.c*")
FILE (GLOB_RECURSE HEADERS "src/lib/*.h*")
FILE (GLOB BIN_SOURCES "src/*.c*")

ADD_LIBRARY (${LIBRARY_NAME} STATIC ${LIB_SOURCES})
IF (TMX_LIB_DIR)
		SET_TARGET_PROPERTIES (${LIBRARY_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${TMX_LIB_DIR}")
ENDIF ()
TARGET_INCLUDE_DIRECTORIES (${LIBRARY_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib>)
TARGET_LINK_LIBRARIES (${LIBRARY_NAME} PUBLIC ${TMXUTILS_LIBRARIES})
       
SET (TMXCTL_LIBRARIES ${PROJECT_NAME}Static PARENT_SCOPE)
       
ADD_EXECUTABLE (${PROJECT_NAME} ${BIN_SOURCES})
IF (TMX_BIN_DIR)
    SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TMX_BIN_DIR}")
ENDIF ()
TARGET_LINK_LIBRARIES (${PROJECT_NAME} PUBLIC ${LIBRARY_NAME})

       
INSTALL (TARGETS ${PROJECT_NAME} EXPORT ${TMX_APPNAME}
         DESTINATION bin COMPONENT ${PROJECT_NAME})
INSTALL (TARGETS ${LIBRARY_NAME} EXPORT ${TMX_APPNAME}
         DESTINATION lib COMPONENT ${PROJECT_NAME})
INSTALL (FILES ${HEADERS} 
         DESTINATION include COMPONENT ${PROJECT_NAME})

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
        ${LIBRARY_NAME}
        ${TMXUTILS_LIBRARIES}
        ${Boost_LIBRARIES}
        gtest
        gmock
        pthread
    )

    # Set C++ standard for test executable
    set_property(TARGET ${BINARY} PROPERTY CXX_STANDARD 14)
    set_property(TARGET ${BINARY} PROPERTY CXX_STANDARD_REQUIRED ON)

    # Include directories for test
    target_include_directories(${BINARY} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
        ${Boost_INCLUDE_DIRS}
    )
endif()
