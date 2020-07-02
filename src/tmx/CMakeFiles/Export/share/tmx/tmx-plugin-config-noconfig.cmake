#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "asn_j2735_r41" for configuration ""
set_property(TARGET asn_j2735_r41 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(asn_j2735_r41 PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libasn_j2735_r41.so"
  IMPORTED_SONAME_NOCONFIG "libasn_j2735_r41.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS asn_j2735_r41 )
list(APPEND _IMPORT_CHECK_FILES_FOR_asn_j2735_r41 "${_IMPORT_PREFIX}/lib/libasn_j2735_r41.so" )

# Import target "asn_j2735_r63" for configuration ""
set_property(TARGET asn_j2735_r63 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(asn_j2735_r63 PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libasn_j2735_r63.so"
  IMPORTED_SONAME_NOCONFIG "libasn_j2735_r63.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS asn_j2735_r63 )
list(APPEND _IMPORT_CHECK_FILES_FOR_asn_j2735_r63 "${_IMPORT_PREFIX}/lib/libasn_j2735_r63.so" )

# Import target "tmxapiStatic" for configuration ""
set_property(TARGET tmxapiStatic APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(tmxapiStatic PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C;CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtmxapi.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS tmxapiStatic )
list(APPEND _IMPORT_CHECK_FILES_FOR_tmxapiStatic "${_IMPORT_PREFIX}/lib/libtmxapi.a" )

# Import target "tmxapi" for configuration ""
set_property(TARGET tmxapi APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(tmxapi PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtmxapi.so"
  IMPORTED_SONAME_NOCONFIG "libtmxapi.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS tmxapi )
list(APPEND _IMPORT_CHECK_FILES_FOR_tmxapi "${_IMPORT_PREFIX}/lib/libtmxapi.so" )

# Import target "tmxutils" for configuration ""
set_property(TARGET tmxutils APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(tmxutils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtmxutils.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS tmxutils )
list(APPEND _IMPORT_CHECK_FILES_FOR_tmxutils "${_IMPORT_PREFIX}/lib/libtmxutils.a" )

# Import target "tmxcore" for configuration ""
set_property(TARGET tmxcore APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(tmxcore PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/tmxcore"
  )

list(APPEND _IMPORT_CHECK_TARGETS tmxcore )
list(APPEND _IMPORT_CHECK_FILES_FOR_tmxcore "${_IMPORT_PREFIX}/bin/tmxcore" )

# Import target "tmxctl" for configuration ""
set_property(TARGET tmxctl APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(tmxctl PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/tmxctl"
  )

list(APPEND _IMPORT_CHECK_TARGETS tmxctl )
list(APPEND _IMPORT_CHECK_FILES_FOR_tmxctl "${_IMPORT_PREFIX}/bin/tmxctl" )

# Import target "tmxctlStatic" for configuration ""
set_property(TARGET tmxctlStatic APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(tmxctlStatic PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtmxctlStatic.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS tmxctlStatic )
list(APPEND _IMPORT_CHECK_FILES_FOR_tmxctlStatic "${_IMPORT_PREFIX}/lib/libtmxctlStatic.a" )

# Import target "j2735dump" for configuration ""
set_property(TARGET j2735dump APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(j2735dump PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/j2735dump"
  )

list(APPEND _IMPORT_CHECK_TARGETS j2735dump )
list(APPEND _IMPORT_CHECK_FILES_FOR_j2735dump "${_IMPORT_PREFIX}/bin/j2735dump" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
