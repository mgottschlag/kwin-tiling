# - Try to find LibXKlavier
# Once done this will define
#
#  LIBXKLAVIER_FOUND - system has LibXKlavier
#  LIBXKLAVIER_LIBRARIES - the libraries needed to use LibXKlavier
#  LIBXKLAVIER_DEFINITIONS - Compiler switches required for using LibXKlavier

if (LIBXKLAVIER_DEFINITIONS AND LIBXKLAVIER_LIBRARIES)

    # in cache already
    SET(LIBXKLAVIER_FOUND TRUE)
  
else (LIBXKLAVIER_DEFINITIONS AND LIBXKLAVIER_LIBRARIES)

    IF (NOT WIN32)
        # use pkg-config to get the directories and then use these values
        # in the FIND_PATH() and FIND_LIBRARY() calls
        INCLUDE(UsePkgConfig)
        PKGCONFIG(libxklavier _LibXKlavierIncDir _LibXKlavierLinkDir _LibXKlavierLinkFlags _LibXKlavierCflags)

	if(_LibXKlavierLinkFlags)
		# find again pkg-config, to query it about libxklavier version
		FIND_PROGRAM(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/bin/ /usr/local/bin )

		# query pkg-config asking for a libxklavier >= 2.91
		EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=2.91 libxklavier RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
		if(_return_VALUE STREQUAL "0")
        		set(LIBXKLAVIER_DEFINITIONS ${_LibXKlavierCflags})
		endif(_return_VALUE STREQUAL "0")
	endif(_LibXKlavierLinkFlags)

    ENDIF (NOT WIN32)

#    FIND_PATH(LIBXKLAVIER_INCLUDE_DIR libxklavier/xklavier.h
#      PATHS
#      ${_LibXKlavierIncDir}
#      PATH_SUFFIXES libxklavier
#    )

    FIND_LIBRARY(LIBXKLAVIER_LIBRARIES NAMES xklavier libxklavier
      PATHS
      ${_LibXKlavierLinkDir}
    )

    if (LIBXKLAVIER_DEFINITIONS AND LIBXKLAVIER_LIBRARIES)
       set(LIBXKLAVIER_FOUND TRUE)
    endif (LIBXKLAVIER_DEFINITIONS AND LIBXKLAVIER_LIBRARIES)

    if (LIBXKLAVIER_FOUND)
      if (NOT LibXKlavier_FIND_QUIETLY)
        message(STATUS "Found LibXKlavier: ${LIBXKLAVIER_LIBRARIES}")
      endif (NOT LibXKlavier_FIND_QUIETLY)
    else (LIBXKLAVIER_FOUND)
      if (LibXKlavier_FIND_REQUIRED)
        message(SEND_ERROR "Could NOT find LibXKlavier")
      endif (LibXKlavier_FIND_REQUIRED)
    endif (LIBXKLAVIER_FOUND)

    MARK_AS_ADVANCED(LIBXKLAVIER_DEFINITIONS LIBXKLAVIER_LIBRARIES)

endif (LIBXKLAVIER_DEFINITIONS AND LIBXKLAVIER_LIBRARIES)
