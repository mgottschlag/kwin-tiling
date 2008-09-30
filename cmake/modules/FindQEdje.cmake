# - Try to find QEdje and QZion
# Once done this will define
#
#  QEDJE_FOUND - system has QEdje
#  QZION_INCLUDE_DIRS - the QZion include directory
#  QEDJE_INCLUDE_DIRS - the QEdje include directory
#  QZION_LIBRARIES - Link these to use QZion
#  QEDJE_LIBRARIES - Link these to use QEdje
#  QZION_CFLAGS_OTHER - Compiler switches required for using QZion
#  QEDJE_CFLAGS_OTHER - Compiler switches required for using QEdje
#

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  INCLUDE(FindPkgConfig)
  PKG_CHECK_MODULES(QEdje qzion>=0.3.0 qedje>=0.3.0)
endif( NOT WIN32 )

# use this just to create a nice message at FindPackageHandleStandardArgs
if (QEdje_FOUND)
  FIND_LIBRARY(QEdje_LIBRARY NAMES qedje)
else (QEdje_FOUND)
  MESSAGE(STATUS "Could not find QZion and/or QEdje.")
endif (QEdje_FOUND)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QEdje DEFAULT_MSG QEdje_LIBRARY)

# show QEdje_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(QEdje_LIBRARY)
