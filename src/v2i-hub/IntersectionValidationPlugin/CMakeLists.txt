PROJECT ( IntersectionValidationPlugin VERSION 7.11.1 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "Intersection Validation")

FIND_PACKAGE (carma-clock)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC  )
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ::carma-clock )

#############
## Testing ##
#############
enable_testing()
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.cpp)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC tmxutils gtest gmock gtest_main)