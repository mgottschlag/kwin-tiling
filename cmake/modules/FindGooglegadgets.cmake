# - Try to find Googlegadgets
# Once done this will define
#
#  GOOGLEGADGETS_FOUND - system has Googlegadgets
#  GOOGLEGADGETS_INCLUDE_DIR - the Googlegadgets include directory
#  GOOGLEGADGETS_LIBRARIES - Link these to use Googlegadgets
#  GOOGLEGADGETS_DEFINITIONS - Compiler switches required for using Googlegadgets
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if ( GOOGLEGADGETS_INCLUDE_DIR AND GOOGLEGADGETS_LIBRARIES )
   # in cache already
   SET(Googlegadgets_FIND_QUIETLY TRUE)
endif ( GOOGLEGADGETS_INCLUDE_DIR AND GOOGLEGADGETS_LIBRARIES )

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  INCLUDE(UsePkgConfig)

  PKGCONFIG(libggadget-1.0 _GooglegadgetsIncDir _GooglegadgetsLinkDir _GooglegadgetsLinkFlags _GooglegadgetsCflags)

  SET(GOOGLEGADGETS_DEFINITIONS ${_GooglegadgetsCflags})
endif( NOT WIN32 )

FIND_PATH(GOOGLEGADGETS_INCLUDE_DIR NAMES ggadget/permissions.h
  PATHS
  ${_GooglegadgetsIncDir}
  PATH_SUFFIXES google-gadgets
)

FIND_LIBRARY(GOOGLEGADGETS_LIBRARIES NAMES ggadget-1.0
  PATHS
  ${_GooglegadgetsLinkDir}
)

FIND_LIBRARY(GOOGLEGADGETSQT_LIBRARIES NAMES ggadget-qt-1.0
  PATHS
  ${_GooglegadgetsLinkDir}
)


include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Googlegadgets DEFAULT_MSG GOOGLEGADGETSQT_LIBRARIES GOOGLEGADGETS_INCLUDE_DIR GOOGLEGADGETS_LIBRARIES )

# show the GOOGLEGADGETS_INCLUDE_DIR and GOOGLEGADGETS_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(GOOGLEGADGETS_INCLUDE_DIR GOOGLEGADGETS_LIBRARIES GOOGLEGADGETSQT_LIBRARIES )

