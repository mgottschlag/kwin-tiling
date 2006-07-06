# - Try to find the PAM libraries
# Once done this will define
#
#  PAM_FOUND - system has pam
#  PAM_INCLUDE_DIR - the pam include directory
#  PAM_LIBRARIES - libpam library

option(KDE4_RPCAUTH "Use Sun's secure RPC for Xauth cookies in kdm." OFF)

if(KDE4_RPCAUTH)
	FIND_PATH(RPC_INCLUDE_DIR rpc/rpc.h
   		/usr/include
   		/usr/local/include
	)
	if(RPC_INCLUDE_DIR)
		set(RPCAUTH_FOUND TRUE)
	else(RPC_INCLUDE_DIR)
		MESSAGE(STATUS "RPC header was not found")
	endif(RPC_INCLUDE_DIR)
endif(KDE4_RPCAUTH)

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

MARK_AS_ADVANCED(PAM_INCLUDE_DIR PAM_LIBRARIES RPC_INCLUDE_DIR)
