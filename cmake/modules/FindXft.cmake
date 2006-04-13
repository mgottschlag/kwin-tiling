# - Try to find Xft
# Once done this will define
#
#  XFT_FOUND - system has Xft
#  XFT_INCLUDE_DIR - the XFT include directory
#  XFT_LIBRARIES - Link these to use XFT
#  XFT_DEFINITIONS - Compiler switches required for using XFT
#


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
INCLUDE(UsePkgConfig)

PKGCONFIG(xft _XFTIncDir _XFTLinkDir _XFTLinkFlags _XFTCflags)
set(XFT_DEFINITIONS ${_XFTCflags})

FIND_LIBRARY(XFT_LIBRARIES NAMES Xft
  PATHS
  ${_XFTLinkDir}
  /usr/lib
  /usr/local/lib
)

if (XFT_LIBRARIES)
   set(XFT_FOUND TRUE)
endif (XFT_LIBRARIES)

if (XFT_FOUND)
  if (NOT XFT_FIND_QUIETLY)
    message(STATUS "Found XFT: ${XFT_LIBRARIES}")
  endif (NOT XFT_FIND_QUIETLY)
else (XFT_FOUND)
  if (XFT_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find XFT")
  endif (XFT_FIND_REQUIRED)
endif (XFT_FOUND)

MARK_AS_ADVANCED(XFT_LIBRARIES)

