#
# V2I Hub Plugins
# Makefile configuration script
#
# Author:
#    Gregory M. Baumgardner
# Version:
#    1.0 : Initial version
#
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS system thread log chrono atomic regex filesystem program_options log_setup) 

function(create_zip output_file input_files working_dir)
    add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E tar "cf" "${output_file}" --format=zip -- ${input_files}
        WORKING_DIRECTORY "${working_dir}"
        OUTPUT  "${output_file}"
        DEPENDS ${input_files}
        COMMENT "Zipping to ${output_file}."
    )
endfunction()

MACRO (BuildTmxPlugin)

    STRING (TOLOWER "${PROJECT_NAME}" COMPONENT_NAME)

    IF (NOT TMX_PLUGIN_NAME)
        SET (TMX_PLUGIN_NAME ${PROJECT_NAME})    
    ENDIF (NOT TMX_PLUGIN_NAME)

    STRING (REGEX REPLACE "[ \t]" "" TMX_PLUGIN_NAME ${TMX_PLUGIN_NAME})
    
    FILE (GLOB_RECURSE SOURCES "src/*.c*")

    ADD_EXECUTABLE (${PROJECT_NAME} ${SOURCES})
    # generate executables in the bin directory to use in a zip file
    SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin")
                      
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json" 
                   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/manifest.json"
                   @ONLY)

    # include the processed manifest.json and executable in the zip file
    ADD_CUSTOM_TARGET ("${PROJECT_NAME}-data" ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.zip)
    FILE(GLOB ZIP_FILES_${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/manifest.json")
    LIST(APPEND ZIP_FILES_${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/${PROJECT_NAME}")
    # create the zip file with the structure that is needed, meaning the plugin
    # name as the directory with manifest.json and bin under this
    create_zip(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.zip "${ZIP_FILES_${PROJECT_NAME}}" ${CMAKE_CURRENT_BINARY_DIR})

    INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.zip
             DESTINATION plugins COMPONENT ${COMPONENT_NAME})

ENDMACRO (BuildTmxPlugin)

IF (NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv71")
    SET (SKIP_LearInterfacePlugin ON)
ENDIF ()

# Get list of available plugins
FILE(GLOB PluginBuilds "${CMAKE_CURRENT_SOURCE_DIR}/*/CMakeLists.txt")
SET(LOG_SKIP_PLUGINS = "")

# Now process the plugins
FOREACH(Plugin ${PluginBuilds})
    GET_FILENAME_COMPONENT(Plugin_Dir ${Plugin} DIRECTORY)
    
    # Do not include the base source directory
    IF("${Plugin_Dir}" STREQUAL "${CMAKE_SOURCE_DIR}")
        CONTINUE()
    ENDIF()
    
    GET_FILENAME_COMPONENT(Plugin_Dir ${Plugin_Dir} NAME)

    # Always build CommandPlugin to connect to web UI
    IF("${Plugin_Dir}" STREQUAL "CommandPlugin")
        MESSAGE(STATUS "Building required plugin: ${Plugin_Dir}")
        ADD_SUBDIRECTORY(${Plugin_Dir})
    # Check if plugin should be built based on user option and skip flag
    ELSEIF(NOT SKIP_${Plugin_Dir})
        MESSAGE(STATUS "Building plugin: ${Plugin_Dir}")
        ADD_SUBDIRECTORY(${Plugin_Dir})
    ELSE()
        MESSAGE(STATUS "Skipping plugin: ${Plugin_Dir}")
        LIST(APPEND LOG_SKIP_PLUGINS ${Plugin_Dir})
    ENDIF()
ENDFOREACH()
MESSAGE(STATUS "Skipped Plugins ${LOG_SKIP_PLUGINS}") 