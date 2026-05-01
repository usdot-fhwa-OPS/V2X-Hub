PROJECT ( IntersectionValidationPlugin VERSION 7.11.1 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "Intersection Validation")

FIND_PACKAGE (carma-clock)

BuildTmxPlugin ()

TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ::carma-clock )

add_library(${PROJECT_NAME}_lib src/MessageIntervalValidator.cpp src/FieldValidation.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC tmxutils)
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${PROJECT_SOURCE_DIR}/src)

#############
## Testing ##
#############
enable_testing()
set(BINARY ${PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.cpp)
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME}_lib gtest gmock gtest_main)