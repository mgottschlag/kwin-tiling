# - Try to find the raw1394 directory library
# Once done this will define
#
#  RAW1394_FOUND - system has RAW1394
#  RAW1394_INCLUDE_DIR - the RAW1394 include directory
#  RAW1394_LIBRARIES - The libraries needed to use FAM

FIND_PATH(RAW1394_INCLUDE_DIR libraw1394/raw1394.h
   /usr/include
   /usr/local/include
)

FIND_LIBRARY(RAW1394_LIBRARIES NAMES raw1394
   PATHS
   /usr/lib
   /usr/local/lib
)


if(RAW1394_INCLUDE_DIR AND RAW1394_LIBRARIES)
   set(RAW1394_FOUND TRUE)
endif(RAW1394_INCLUDE_DIR AND RAW1394_LIBRARIES)


if(RAW1394_FOUND)
   if(NOT RAW1394_FIND_QUIETLY)
      message(STATUS "Found raw1394: ${RAW1394_LIBRARIES}")
   endif(NOT RAW1394_FIND_QUIETLY)
endif(RAW1394_FOUND)

MARK_AS_ADVANCED(RAW1394_INCLUDE_DIR RAW1394_LIBRARIES)

