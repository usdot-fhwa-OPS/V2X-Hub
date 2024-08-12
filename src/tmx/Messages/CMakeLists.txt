PROJECT ( tmxmessages NONE )

SET (TMXMESSAGES_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

INSTALL (DIRECTORY include
         DESTINATION . COMPONENT ${PROJECT_NAME}
         FILES_MATCHING PATTERN "*.h*")

#############
## Testing ##
#############
enable_testing()
 
set(BINARY ${PROJECT_NAME}_test)
 
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
 
add_executable(${BINARY} ${TEST_SOURCES})
 
add_test(NAME ${BINARY} COMMAND ${BINARY})
 
target_link_libraries(${BINARY} PUBLIC ${TMXAPI_LIBRARIES} gtest)