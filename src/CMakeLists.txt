CMAKE_MINIMUM_REQUIRED (VERSION 3.10.2)
project(V2XHub VERSION 7.11.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(tmx)
add_subdirectory(v2i-hub)
add_dependencies(v2i-hub tmx)

add_custom_target(build_all DEPENDS tmx v2i-hub)
