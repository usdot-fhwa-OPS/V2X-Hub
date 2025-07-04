PROJECT ( tmxapi C CXX )

FILE (GLOB_RECURSE TMXAPI_SOURCES "tmx/*.c*")

SET (TMXAPI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Configure Boost for TMX use
IF (NOT Boost_LIBRARIES)
    SET (Boost_USE_MULTITHREADED ON)

    SET (Boost_DEFS "BOOST_ASIO_HEADER_ONLY -DBOOST_SPIRIT_THREADSAFE ${XTRA_BOOST_DEFS}")
    IF (NOT Boost_USE_STATIC_RUNTIME)
        SET (Boost_DEFS "${Boost_DEFS} -DBOOST_ALL_DYN_LINK")
    ENDIF ()
    
    IF (NOT BOOST_MIN_VERSION)
        SET (BOOST_MIN_VERSION 1.58)
    ENDIF (NOT BOOST_MIN_VERSION)
    
    FIND_PACKAGE ( Boost ${BOOST_MIN_VERSION}
                   COMPONENTS system thread atomic chrono 
                              regex log log_setup filesystem 
                              program_options 
                              ${XTRA_BOOST_LIBRARIES} REQUIRED )
    
    SET (Boost_VERSION "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}" CACHE INTERNAL "Boost")                             
ENDIF (NOT Boost_LIBRARIES)

# Configure internationalization libraries needed at least for Boost regex
IF (NOT ICU_LIBRARIES)
    FIND_LIBRARY (ICUI18_LIBRARY icui18n)
    FIND_LIBRARY (ICUUC_LIBRARY icuuc)
    FIND_LIBRARY (ICUDATA_LIBRARY icudata)

    SET (ICU_LIBRARIES ${ICUI18_LIBRARY} ${ICUUC_LIBRARY} ${ICUDATA_LIBRARY})
ENDIF (NOT ICU_LIBRARIES)

IF (NOT TMXAPI_NO_STATIC)
    ADD_LIBRARY (${PROJECT_NAME}Static STATIC ${TMXAPI_SOURCES})
    SET_TARGET_PROPERTIES (${PROJECT_NAME}Static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    IF (TMX_LIB_DIR)
        SET_TARGET_PROPERTIES (${PROJECT_NAME}Static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${TMX_LIB_DIR}")
    ENDIF ()
    TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME}Static SYSTEM PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                /opt/carma/include/stol-j2735-${SAEJ2735_SPEC_FULL_VERSION}-carma
                                ${Boost_INCLUDE_DIRS}) 
    TARGET_LINK_LIBRARIES (${PROJECT_NAME}Static ${J2735_LIBRARY_NAME} ${Boost_LIBRARIES} ${ICU_LIBRARIES} pugixml)
    TARGET_COMPILE_DEFINITIONS (${PROJECT_NAME}Static PUBLIC "-DSAEJ2735_SPEC=${SAEJ2735_SPEC_VERSION}")

    IF (Boost_DEFS)
        TARGET_COMPILE_DEFINITIONS (${PROJECT_NAME}Static PUBLIC ${Boost_DEFS})
    ENDIF (Boost_DEFS)
    IF (NOT TMXAPI_NO_COMPILE_FEATURES)
        TARGET_COMPILE_FEATURES (${PROJECT_NAME}Static PUBLIC
                                 cxx_lambdas cxx_nullptr cxx_range_for cxx_variadic_templates)
    ENDIF ()
                             
    INSTALL (TARGETS ${PROJECT_NAME}Static EXPORT ${TMX_APPNAME} ARCHIVE 
             DESTINATION lib COMPONENT lib${PROJECT_NAME})
    
    SET (TMXAPI_LIBRARIES ${PROJECT_NAME}Static PARENT_SCOPE)
ENDIF (NOT TMXAPI_NO_STATIC) 

# Always prefer to link against shared object, so it is ok to overwrite variables here
IF (NOT TMXAPI_NO_SHARED)
    ADD_LIBRARY (${PROJECT_NAME} SHARED ${TMXAPI_SOURCES})
    IF (TMX_LIB_DIR)
        SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TMX_LIB_DIR}")
    ENDIF ()
    TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME} SYSTEM PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                /opt/carma/include/stol-j2735-${SAEJ2735_SPEC_FULL_VERSION}-carma
                                ${Boost_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES (${PROJECT_NAME} PUBLIC ${J2735_LIBRARY_NAME} ${Boost_LIBRARIES} ${ICU_LIBRARIES} pugixml)
    TARGET_COMPILE_DEFINITIONS (${PROJECT_NAME} PUBLIC "-DSAEJ2735_SPEC=${SAEJ2735_SPEC_VERSION}")
    IF (Boost_DEFS)
        TARGET_COMPILE_DEFINITIONS (${PROJECT_NAME} PUBLIC ${Boost_DEFS})
    ENDIF (Boost_DEFS)
    IF (NOT TMXAPI_NO_COMPILE_FEATURES)
        TARGET_COMPILE_FEATURES (${PROJECT_NAME} PUBLIC
                                 cxx_lambdas cxx_nullptr cxx_range_for cxx_variadic_templates )
    ENDIF ()
    
    INSTALL (TARGETS ${PROJECT_NAME} EXPORT ${TMX_APPNAME} 
	     ARCHIVE DESTINATION lib
	     LIBRARY DESTINATION lib COMPONENT lib${PROJECT_NAME})
    
    SET (TMXAPI_LIBRARIES ${PROJECT_NAME} PARENT_SCOPE)
ENDIF (NOT TMXAPI_NO_SHARED)

INSTALL (DIRECTORY tmx 
         DESTINATION include COMPONENT lib${PROJECT_NAME}
         FILES_MATCHING PATTERN "*.h*" PATTERN ".*" EXCLUDE)
         
IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/treerepair.xml")
	INSTALL (FILES "${CMAKE_CURRENT_SOURCE_DIR}/treerepair.xml"
		DESTINATION share/xml/misc COMPONENT lib${PROJECT_NAME})
ENDIF ()
		 
