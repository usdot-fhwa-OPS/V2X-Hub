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
set(CMAKE_CXX_STANDARD 14)

# Global values
SET (TMX_APPNAME tmx CACHE INTERNAL "Application name")
SET (TMX_APPDESCR "V2I Hub" CACHE INTERNAL "Application description")

# Global values
IF (TMX_VERBOSE)
    SET(CMAKE_VERBOSE_MAKEFILE ON CACHE INTERNAL "C Project setting")
    SET(CMAKE_USE_RELATIVE_PATHS ON CACHE INTERNAL "C Project setting")
ENDIF (TMX_VERBOSE)

ENABLE_LANGUAGE ( C )
ENABLE_LANGUAGE ( CXX )

INCLUDE (CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG ("-fPIC" CMAKE_C_FLAG_FPIC_SUPPORTED)

IF (NOT TMX_BIN_DIR)
    SET (TMX_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
ENDIF (NOT TMX_BIN_DIR)

IF (NOT TMX_LIB_DIR)
    SET (TMX_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
ENDIF (NOT TMX_LIB_DIR)

IF (TMX_CROSSCOMPILING)
    MESSAGE (STATUS "Cross-compiling for ${CMAKE_SYSTEM} on ${CMAKE_SYSTEM_PROCESSOR}")
ENDIF (TMX_CROSSCOMPILING)

LINK_DIRECTORIES (${PROJECT_NAME} "${TMX_LIB_DIR}")

SET (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}")

GET_FILENAME_COMPONENT (BUILD_SUB_DIR "${CMAKE_BINARY_DIR}" NAME)

IF (TMX_DIR AND EXISTS "${TMX_DIR}")
    SET (tmx-plugin_DIR ${TMX_DIR})
ENDIF ()

FIND_PACKAGE (${TMX_APPNAME}-plugin
              HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../${TMX_APPNAME}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../${TMX_APPNAME}/Build
              PATH_SUFFIXES ${BUILD_SUB_DIR} ${BUILD_SUB_DIR}/${CMAKE_SYSTEM_PROCESSOR}
              REQUIRED)

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
    # include the processed manifest.json and executable in the zip file
    ADD_CUSTOM_TARGET ("${PROJECT_NAME}-data" ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.zip)
    FILE(GLOB ZIP_FILES_${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/manifest.json")
    LIST(APPEND ZIP_FILES_${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/${PROJECT_NAME}")
    # create the zip file with the structure that is needed, meaning the plugin
    # name as the directory with manifest.json and bin under this
    create_zip(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.zip "${ZIP_FILES_${PROJECT_NAME}}" ${CMAKE_CURRENT_BINARY_DIR})

    INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.zip
             DESTINATION plugins COMPONENT ${COMPONENT_NAME})
                      
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json" 
                   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/manifest.json"
                   @ONLY)

ENDMACRO (BuildTmxPlugin)

IF (NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv71")
	SET (SKIP_LearInterfacePlugin ON)
ENDIF ()

FILE (GLOB PluginBuilds "${CMAKE_CURRENT_SOURCE_DIR}/*/CMakeLists.txt")

FOREACH (Plugin ${PluginBuilds})
    GET_FILENAME_COMPONENT (Plugin_Dir ${Plugin} DIRECTORY)
    
    # Do not include the base source directory, or else it might recurse forever
    IF ("${Plugin_Dir}" STREQUAL "${CMAKE_SOURCE_DIR}")
        CONTINUE ()
    ENDIF ()
    
    GET_FILENAME_COMPONENT (Plugin_Dir ${Plugin_Dir} NAME)
    
    IF (NOT SKIP_${Plugin_Dir})
        ADD_SUBDIRECTORY (${Plugin_Dir})
    ENDIF ()
ENDFOREACH ()
