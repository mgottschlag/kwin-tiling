# Copyright 2012 Lamarque V. Souza <lamarque@kde.org>
# Find the native KDeclarative includes and library
# This module defines
#  KDECLARATIVE_INCLUDE_DIR, where to find kdeclarative.h
#  KDECLARATIVE_LIBRARIES, the libraries needed to use KDeclarative.
#  KDECLARATIVE_FOUND, If false, do not try to use KDeclarative.
# also defined, but not for general use are
#  KDECLARATIVE_LIBRARY, where to find the KDeclarative library.

#=============================================================================
# Copyright 2012 Lamarque V. Souza <lamarque@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PATH(KDECLARATIVE_INCLUDE_DIR kdeclarative.h)

FIND_LIBRARY(KDECLARATIVE_LIBRARY NAMES kdeclarative )

# handle the QUIETLY and REQUIRED arguments and set KDECLARATIVE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(${kde_cmake_module_dir}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KDECLARATIVE DEFAULT_MSG KDECLARATIVE_LIBRARY KDECLARATIVE_INCLUDE_DIR)

IF(KDECLARATIVE_FOUND)
  SET(KDECLARATIVE_LIBRARIES ${KDECLARATIVE_LIBRARY})
ENDIF(KDECLARATIVE_FOUND)

MARK_AS_ADVANCED(KDECLARATIVE_LIBRARY KDECLARATIVE_LIBRARIES KDECLARATIVE_INCLUDE_DIR )
