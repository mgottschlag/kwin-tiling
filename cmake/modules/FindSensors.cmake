# - Try to find the sensors directory library
# Once done this will define
#
#  SENSORS_FOUND - system has SENSORS
#  SENSORS_INCLUDE_DIR - the SENSORS include directory
#  SENSORS_LIBRARIES - The libraries needed to use SENSORS

FIND_PATH(SENSORS_INCLUDE_DIR sensors/sensors.h
   /usr/include
   /usr/local/include
)

FIND_LIBRARY(SENSORS_LIBRARIES NAMES sensors
   PATHS
   /usr/lib
   /usr/local/lib
)


if(SENSORS_INCLUDE_DIR AND SENSORS_LIBRARIES)
   set(SENSORS_FOUND TRUE)
endif(SENSORS_INCLUDE_DIR AND SENSORS_LIBRARIES)


if(SENSORS_FOUND)
   if(NOT Sensors_FIND_QUIETLY)
      message(STATUS "Found sensors: ${SENSORS_LIBRARIES}")
   endif(NOT Sensors_FIND_QUIETLY)
endif(SENSORS_FOUND)

MARK_AS_ADVANCED(SENSORS_INCLUDE_DIR SENSORS_LIBRARIES)

