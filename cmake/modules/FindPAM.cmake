# - Try to find the PAM libraries
# Once done this will define
#
#  PAM_FOUND - system has pam
#  PAM_INCLUDE_DIR - the pam include directory
#  PAM_LIBRARIES - libpam library

if(PAM_INCLUDE_DIR AND PAM_LIBRARIES)
    # Already in cache, be silent
    set(PAM_FIND_QUIETLY TRUE)
endif(PAM_INCLUDE_DIR AND PAM_LIBRARIES)


FIND_PATH(PAM_INCLUDE_DIR security/pam_appl.h
   /usr/include
   /usr/local/include
)

if( NOT PAM_INCLUDE_DIR)
	FIND_PATH(PAM_INCLUDE_DIR pam/pam_appl.h
		/usr/include
		/usr/local/include
	)
endif(NOT PAM_INCLUDE_DIR) 

FIND_LIBRARY(PAM_LIBRARIES NAMES pam
	PATHS
	/usr/lib
	/usr/local/lib
)


if(PAM_INCLUDE_DIR AND PAM_LIBRARIES)
   set(PAM_FOUND TRUE)
endif(PAM_INCLUDE_DIR AND PAM_LIBRARIES)

if(PAM_FOUND)
   if(NOT PAM_FIND_QUIETLY)
      message(STATUS "Found pam: ${PAM_LIBRARIES} ")
   endif(NOT PAM_FIND_QUIETLY)
else(PAM_FOUND)
	MESSAGE(STATUS "Pam was not found")
endif(PAM_FOUND)

MARK_AS_ADVANCED(PAM_INCLUDE_DIR PAM_LIBRARIES)
