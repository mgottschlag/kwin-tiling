# Copyright 2012 Lamarque V. Souza <lamarque@kde.org>
# Find the native KDeclarative includes and library
# This module defines
#  KDECLARATIVE_INCLUDE_DIR, where to find kdeclarative.h
#  KDECLARATIVE_LIBRARIES, the libraries needed to use KDeclarative.
#  KDECLARATIVE_FOUND, If false, do not try to use KDeclarative.
# also defined, but not for general use are
#  KDECLARATIVE_LIBRARY, where to find the KDeclarative library.

#=============================================================================

FIND_PATH(KDECLARATIVE_INCLUDE_DIR kdeclarative.h)

SET(KDECLARATIVE_NAMES ${KDECLARATIVE_NAMES} kdeclarative)
FIND_LIBRARY(KDECLARATIVE_LIBRARY NAMES ${KDECLARATIVE_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set KDECLARATIVE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${kde_cmake_module_dir}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KDECLARATIVE DEFAULT_MSG KDECLARATIVE_LIBRARY KDECLARATIVE_INCLUDE_DIR)

IF(KDECLARATIVE_FOUND)
  SET(KDECLARATIVE_LIBRARIES ${KDECLARATIVE_LIBRARY})
ENDIF(KDECLARATIVE_FOUND)

# Deprecated declarations.
SET (NATIVE_KDECLARATIVE_INCLUDE_PATH ${KDECLARATIVE_INCLUDE_DIR} )
IF(KDECLARATIVE_LIBRARY)
  GET_FILENAME_COMPONENT (NATIVE_KDECLARATIVE_LIB_PATH ${KDECLARATIVE_LIBRARY} PATH)
ENDIF(KDECLARATIVE_LIBRARY)

MARK_AS_ADVANCED(KDECLARATIVE_LIBRARY KDECLARATIVE_INCLUDE_DIR )
