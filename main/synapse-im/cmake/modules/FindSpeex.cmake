# - Try to find the Speex coding library
# Once done this will define
#
#  SPEEX_FOUND - system has the Speex library
#  SPEEX_INCLUDE_DIR - the Speex include directory
#  SPEEX_LIBRARIES - The libraries needed to use Speex
# Copyright (c) 2007, Andrzej Wojcik, <andrzej@hi-low.eu>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# on win32 we additional need to link to libeay32.lib

IF(SPEEX_LIBRARIES)
   SET(Speex_FIND_QUIETLY TRUE)
ENDIF(SPEEX_LIBRARIES)

FIND_PATH(SPEEX_INCLUDE_DIR speex/speex.h )

FIND_LIBRARY(SPEEX_LIBRARIES NAMES speex)

IF(SPEEX_INCLUDE_DIR AND SPEEX_LIBRARIES)
    SET(SPEEX_FOUND TRUE)
ELSE(SPEEX_INCLUDE_DIR AND SPEEX_LIBRARIES)
    SET(SPEEX_FOUND FALSE)
ENDIF (SPEEX_INCLUDE_DIR AND SPEEX_LIBRARIES)

MARK_AS_ADVANCED(SPEEX_INCLUDE_DIR SPEEX_LIBRARIES)

