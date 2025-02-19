PROJECT ( CDA1TenthPlugin VERSION 7.6.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "CDA1Tenth")
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wno-unused-variable")


FIND_PATH (WEBSOCKETS_INCLUDE_DIR lws_config.h)

SET (CMAKE_LIBRARY_PATH ${WEBSOCKETS_INCLUDE_DIR}/../lib ${CMAKE_LIBRARY_PATH})
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(OpenSSL REQUIRED)
FIND_LIBRARY (EV_LIBRARY ev)
FIND_LIBRARY (UV_LIBRARY uv)
FIND_LIBRARY (CAP_LIBRARY cap)
find_library(WEBSOCKETS_LIBRARY libwebsockets.a)

include_directories( "/usr/local/include/pdclient")

BuildTmxPlugin ( )

target_link_libraries(${PROJECT_NAME} PRIVATE pdclient )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Network ssl crypto ) 
target_link_libraries(${PROJECT_NAME} PUBLIC tmxutils ${WEBSOCKETS_LIBRARY} ${OPENSSL_LIBRARIES} ${EV_LIBRARY} ${UV_LIBRARY} ${CAP_LIBRARY})

#############
## Testing ##
#############
enable_testing()
add_library(${PROJECT_NAME}_lib src/BSMConverter.cpp
                                src/ActionConverter.cpp
                                src/MobilityOperationConverter.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC 
                                        tmxutils)
target_include_directories(${PROJECT_NAME}_lib PUBLIC 
                                        ${PROJECT_SOURCE_DIR}/src)
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest)

