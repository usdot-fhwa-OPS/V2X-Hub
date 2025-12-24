project( TimPlugin VERSION 7.11.0 LANGUAGES CXX )

set(TMX_PLUGIN_NAME "TIM")
# Required for QT libraries
add_compile_options(-fPIC)
set(CMAKE_CXX_STANDARD 17)

find_package(XercesC REQUIRED)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

find_package(qserverPedestrian REQUIRED)

BuildTmxPlugin()

target_include_directories( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})

target_link_libraries( ${PROJECT_NAME} PUBLIC tmxutils ${XercesC_LIBRARY} ${QHttpEngine_LIBRARY} Qt5Widgets Qt5Core Qt5Network ssl crypto qhttpengine qserverPedestrian) 

####################################################
##################  Testing  #######################
####################################################

add_library(${PROJECT_NAME}_lib 
    src/TimUtils.cpp
    )
target_link_libraries(${PROJECT_NAME}_lib PUBLIC  tmxutils )
enable_testing()
include_directories(${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*.h test/*.cpp)
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_test PRIVATE gtest gtest_main ${PROJECT_NAME}_lib)


