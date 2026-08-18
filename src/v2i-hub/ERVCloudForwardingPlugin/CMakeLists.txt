PROJECT(ERVCloudForwardingPlugin VERSION 7.11.1 LANGUAGES CXX)

SET(TMX_PLUGIN_NAME "ERVCloudForwarding")
add_compile_options(-fPIC)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(CURL REQUIRED)


find_library(libasn1c .)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
)

BuildTmxPlugin()

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC  ${Qt5Core_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC tmxutils 
   ${QHttpEngine_LIBRARY} ${Boost_SYSTEM_LIBRARY} NemaTode curl Qt5Core Qt5Network qhttpengine v2xhubWebAPI)
link_directories(${CMAKE_PREFIX_PATH}/lib)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:ALLOW_INSECURE_TLS=1>
)

# ############
# # Testing ##
# ############
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
add_library(${PROJECT_NAME}_lib src/ERVCloudForwardingWorker.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC NemaTode ${TMXAPI_LIBRARIES}
   tmxutils )
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
set(SOURCES ${TEST_SOURCES} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest gtest_main)