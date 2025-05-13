PROJECT ( FLIRCameraDriverPlugin VERSION 7.6.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "FLIRCameraDriver")
set(CMAKE_CXX_STANDARD 17)
find_package(carma-clock REQUIRED)
BuildTmxPlugin()
target_link_libraries(${PROJECT_NAME} tmxutils ::carma-clock)


