PROJECT ( CDA1TenthPlugin VERSION 7.6.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "CDA1Tenth")
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wno-unused-variable")

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)


include_directories( "/usr/local/include/pdclient")

BuildTmxPlugin ( )

target_link_libraries(${PROJECT_NAME} PRIVATE pdclient )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Network ssl crypto ) 
target_link_libraries(${PROJECT_NAME} PUBLIC tmxutils)

#############
## Testing ##
#############
enable_testing()
add_library(${PROJECT_NAME}_lib src/BSMConverter.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC 
                                        tmxutils)
target_include_directories(${PROJECT_NAME}_lib PUBLIC 
                                        ${PROJECT_SOURCE_DIR}/src)
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest)

