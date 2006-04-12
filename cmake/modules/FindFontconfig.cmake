# - Try to find the  Fontconfig
# Once done this will define
#
#  FONTCONFIG_FOUND - system has Fontconfig
#  FONTCONFIG_INCLUDE_DIR - the FONTCONFIG include directory
#  FONTCONFIG_LIBRARIES - Link these to use FONTCONFIG
#  FONTCONFIG_DEFINITIONS - Compiler switches required for using FONTCONFIG
#


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
INCLUDE(UsePkgConfig)

PKGCONFIG(fontconfig _FONTCONFIGIncDir _FONTCONFIGLinkDir _FONTCONFIGLinkFlags _FONTCONFIGCflags)
set(FONTCONFIG_DEFINITIONS ${_FONTCONFIGCflags})

FIND_LIBRARY(FONTCONFIG_LIBRARIES NAMES fontconfig
  PATHS
  ${_FONTCONFIGLinkDir}
  /usr/lib
  /usr/local/lib
)

if (FONTCONFIG_LIBRARIES)
   set(FONTCONFIG_FOUND TRUE)
endif (FONTCONFIG_LIBRARIES)

if (FONTCONFIG_FOUND)
  if (NOT FONTCONFIG_FIND_QUIETLY)
    message(STATUS "Found FONTCONFIG: ${FONTCONFIG_LIBRARIES}")
  endif (NOT FONTCONFIG_FIND_QUIETLY)
else (FONTCONFIG_FOUND)
  if (FONTCONFIG_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find FONTCONFIG")
  endif (FONTCONFIG_FIND_REQUIRED)
endif (FONTCONFIG_FOUND)

MARK_AS_ADVANCED(FONTCONFIG_LIBRARIES)

