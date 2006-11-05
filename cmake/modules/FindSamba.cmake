# - Try to find the samba directory library
# Once done this will define
#
#  SAMBA_FOUND - system has SAMBA
#  SAMBA_INCLUDE_DIR - the SAMBA include directory
#  SAMBA_LIBRARIES - The libraries needed to use SAMBA
#  SAMBA_HAVE_SMBC_SET_CONTEXT - true if libsmbclient has smbc_set_context()

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)
    # Already in cache, be silent
    set(SAMBA_FIND_QUIETLY TRUE)
endif(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)

find_path(SAMBA_INCLUDE_DIR NAMES libsmbclient.h )

find_library(SAMBA_LIBRARIES NAMES smbclient )


if(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)
   set(SAMBA_FOUND TRUE)
   # check whether libsmbclient has smbc_set_context()
   include(CheckSymbolExists)
   include(MacroPushRequiredVars)
   macro_push_required_vars()
   set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${SAMBA_LIBRARIES})
   check_symbol_exists(smbc_set_context "libsmbclient.h" SAMBA_HAVE_SMBC_SET_CONTEXT)
   macro_pop_required_vars()
else(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)
   set(SAMBA_FOUND FALSE)
   set(SAMBA_HAVE_SMBC_SET_CONTEXT FALSE)
endif(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)


if(SAMBA_FOUND)
   if(NOT Samba_FIND_QUIETLY)
      message(STATUS "Found samba: ${SAMBA_LIBRARIES}")
   endif(NOT Samba_FIND_QUIETLY)
else(SAMBA_FOUND)
   if (Samba_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Samba library")
   endif (Samba_FIND_REQUIRED)
endif(SAMBA_FOUND)

mark_as_advanced(SAMBA_INCLUDE_DIR SAMBA_LIBRARIES)

