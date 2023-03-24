# - Try to find MySQL Connector and Client libraries
# Once done, this will define the following variables:
#   LIBMYSQLCLIENT_FOUND - System has or does not have MySQL client libraries
#   LIBMYSQLCLIENT_INCLUDE_DIR - The MySQL Client include directories
#   LIBMYSQLCLIENT_LIBRARIES - The MySQL Client libraries
#   LIBMYSQLCLIENT_DEFINITIONS - Compiler switches required for using MySQL Client libraries
#
#   LIBMYSQLCONNECTOR_C_FOUND - System has or does not have MySQL connector libraries for C
#   LIBMYSQLCONNECTOR_C_INCLUDE_DIR - The MySQL connector include directories for C
#   LIBMYSQLCONNECTOR_C_LIBRARIES - The MySQL connector libraries for C
#   LIBMYSQLCONNECTOR_C_DEFINITIONS - Compiler switches required for using MySQL connector libraries for C
#
#   LIBMYSQLCONNECTOR_CXX_FOUND - System has or does not have MySQL connector libraries for C++
#   LIBMYSQLCONNECTOR_CXX_INCLUDE_DIR - The MySQL connector include directories for C++
#   LIBMYSQLCONNECTOR_CXX_LIBRARIES - The MySQL connector libraries for C++
#   LIBMYSQLCONNECTOR_CXX_DEFINITIONS - Compiler switches required for using MySQL connector libraries for C++


# - Find mysqlclient
#
# -*- cmake -*-
#
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.

IF (MYSQL_INCLUDE_DIR)
  # Already in cache, be silent
  SET(MYSQL_FIND_QUIETLY TRUE)
ENDIF (MYSQL_INCLUDE_DIR)

FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
  /usr/local/include/mysql
  /usr/include/mysql
)

SET(MYSQL_NAMES mysqlclient mysqlclient_r)
FIND_LIBRARY(MYSQL_LIBRARY
  NAMES ${MYSQL_NAMES}
  PATHS /usr/lib /usr/local/lib
  PATH_SUFFIXES mysql
)

IF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND TRUE)
  SET( MYSQL_LIBRARIES ${MYSQL_LIBRARY} )
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND FALSE)
  SET( MYSQL_LIBRARIES )
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)

IF (MYSQL_FOUND)
  IF (NOT MYSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Found MySQL: ${MYSQL_LIBRARY}")
  ENDIF (NOT MYSQL_FIND_QUIETLY)
ELSE (MYSQL_FOUND)
  IF (MYSQL_FIND_REQUIRED)
    MESSAGE(STATUS "Looked for MySQL libraries named ${MYSQL_NAMES}.")
    MESSAGE(FATAL_ERROR "Could NOT find MySQL library")
  ENDIF (MYSQL_FIND_REQUIRED)
ENDIF (MYSQL_FOUND)

MARK_AS_ADVANCED(
  MYSQL_LIBRARY
  MYSQL_INCLUDE_DIR
  )

#  
# Added by GMB
# 
# For including the MySQL C++ Connector library
#
# Note that on Cygwin, this library may be stored in a pre-compiled area
#


GET_FILENAME_COMPONENT (MYSQL_LIBRARY_DIR ${MYSQL_LIBRARY} DIRECTORY)
  
FIND_PATH ( MYSQLCPPCONN_INCLUDE_DIR cppconn/driver.h HINTS ${MYSQL_INCLUDE_DIR} ${INC_HINTS} )
FIND_LIBRARY ( MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn HINTS ${MYSQL_LIBRARY_DIR} ${LIB_HINTS} )

SET (MYSQLCPPCONN_LIBRARIES ${MYSQLCPPCONN_LIBRARY} )
SET (MYSQLCPPCONN_INCLUDE_DIRS ${MYSQLCPPCONN_INCLUDE_DIR} )

MESSAGE (STATUS "Found MySQL C++ Connector: ${MYSQLCPPCONN_LIBRARY}")
INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (TmxMySQL DEFAULT_MSG MYSQLCPPCONN_INCLUDE_DIR MYSQLCPPCONN_LIBRARY)

MARK_AS_ADVANCED ( MYSQLCPPCONN_LIBRARY MYSQLCPPCONN_INCLUDE_DIR )
