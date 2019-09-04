# - Try to find capabilities
# Find the native CAP includes and library
#
#  CAP_FOUND - system has capabilities available
#  CAP_INCLUDE_DIRS - where to find capabilities.h
#
#  Copyright (c) 2019 6WIND
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (CAP_INCLUDE_DIRS)
  # Already in cache, be silent
  SET(CAP_FIND_QUIETLY TRUE)
ENDIF (CAP_INCLUDE_DIRS)

find_package(PkgConfig)
pkg_search_module(CAP libcap)

FIND_PATH(CAP_INCLUDE_DIR sys/capability.h)
  HINTS ${CAP_LIBDIR}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CAP DEFAULT_MSG CAP_INCLUDE_DIR)

IF(CAP_FOUND)
  SET( CAP_INCLUDE_DIRS ${CAP_INCLUDE_DIR} )
ELSE(CAP_FOUND)
  SET( CAP_INCLUDE_DIRS)
ENDIF(CAP_FOUND)

MARK_AS_ADVANCED( CAP_INCLUDE_DIRS )
