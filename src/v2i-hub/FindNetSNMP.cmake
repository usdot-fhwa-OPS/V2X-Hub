
# - Find Net-SNMP
#
# -*- cmake -*-
#
# Find the Net-SNMP module
#
#  NETSNMP_INCLUDE_DIR - where to find Net-SNMP.h, etc.
#  NETSNMP_LIBRARIES   - List of libraries when using Net-SNMP.
#  NETSNMP_FOUND       - True if Net-SNMP found.

IF (NETSNMP_INCLUDE_DIR)
  # Already in cache, be silent
  SET(NETSNMP_FIND_QUIETLY TRUE)
ENDIF (NETSNMP_INCLUDE_DIR)

FIND_PATH(NETSNMP_INCLUDE_DIR snmp.h
  PATHS /usr/local/include/net-snmp/library
  /usr/include/net-snmp/library
)

SET(NETSNMP_NAMES netsnmp)
FIND_LIBRARY(NETSNMP_LIBRARY
  NAMES ${NETSNMP_NAMES}
  PATHS /usr/lib /usr/local/lib
)

SET(NETSNMPAGENT_NAMES netsnmpagent)
FIND_LIBRARY(NETSNMPAGENT_LIBRARY
  NAMES ${NETSNMPAGENT_NAMES}
  PATHS /usr/lib /usr/local/lib
)

SET(NETSNMPHELPERS_NAMES netsnmphelpers)
FIND_LIBRARY(NETSNMPHELPERS_LIBRARY
  NAMES ${NETSNMPHELPERS_NAMES}
  PATHS /usr/lib /usr/local/lib
)

SET(NETSNMPMIBS_NAMES netsnmpmibs)
FIND_LIBRARY(NETSNMPMIBS_LIBRARY
  NAMES ${NETSNMPMIBS_NAMES}
  PATHS /usr/lib /usr/local/lib
)

SET(NETSNMPTRAPD_NAMES netsnmptrapd)
FIND_LIBRARY(NETSNMPTRAPD_LIBRARY
  NAMES ${NETSNMPTRAPD_NAMES}
  PATHS /usr/lib /usr/local/lib
)

SET(NETSNMP_LIBRARIES
  ${NETSNMP_LIBRARY}
  ${NETSNMPAGENT_LIBRARY}
  ${NETSNMPHELPERS_LIBRARY}
  ${NETSNMPMIBS_LIBRARY}
#  ${NETSNMPTRAPD_LIBRARY}
)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NetSNMP
  DEFAULT_MSG
  NETSNMP_INCLUDE_DIR
  NETSNMP_LIBRARIES
)

MARK_AS_ADVANCED(
  NETSNMP_LIBRARY
  NETSNMPAGENT_LIBRARY
  NETSNMPHELPERS_LIBRARY
  NETSNMPMIBS_LIBRARY
  NETSNMPTRAPD_LIBRARY
  NETSNMP_INCLUDE_DIR
  )
