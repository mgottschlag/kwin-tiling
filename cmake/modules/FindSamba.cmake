# - Try to find the samba directory library
# Once done this will define
#
#  SAMBA_FOUND - system has SAMBA
#  SAMBA_INCLUDE_DIR - the SAMBA include directory
#  SAMBA_LIBRARIES - The libraries needed to use SAMBA

if(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)
    # Already in cache, be silent
    set(SAMBA_FIND_QUIETLY TRUE)
endif(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)

FIND_PATH(SAMBA_INCLUDE_DIR libsmbclient.h
   /usr/include
   /usr/local/include
)

FIND_LIBRARY(SAMBA_LIBRARIES NAMES smbclient
   PATHS
   /usr/lib
   /usr/local/lib
)


if(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)
   set(SAMBA_FOUND TRUE)
endif(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)


if(SAMBA_FOUND)
   if(NOT SAMBA_FIND_QUIETLY)
      message(STATUS "Found samba: ${SAMBA_LIBRARIES}")
   endif(NOT SAMBA_FIND_QUIETLY)
endif(SAMBA_FOUND)

MARK_AS_ADVANCED(SAMBA_INCLUDE_DIR SAMBA_LIBRARIES)

