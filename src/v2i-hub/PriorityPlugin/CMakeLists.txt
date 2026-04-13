PROJECT ( PriorityPlugin VERSION 7.11.1 LANGUAGES CXX )
SET (CMAKE_CXX_STANDARD 17)
SET (TMX_PLUGIN_NAME "Priority")

FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES} pthread)

#############
## Testing ##
#############
enable_testing()
find_package(GTest REQUIRED)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(${PROJECT_NAME}_lib src/PriorityRequestProcessor.cpp)
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC tmxutils)

set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib tmxutils gtest gtest_main pthread)
