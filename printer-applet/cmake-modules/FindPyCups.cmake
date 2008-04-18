# Find PyCups
# ~~~~~~~~~~
# Copyright (c) 2008, Jonathan Riddell <jriddell@ubuntu.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# PyCups website: http://cyberelk.net/tim/software/pycups/
#
# Find the installed version of PyCups.

IF(PYCUPS_FOUND)
  # Already in cache, be silent
  SET(PYCUPS_FOUND TRUE)
ELSE(PYCUPS_FOUND)

  GET_FILENAME_COMPONENT(_cmake_module_path ${CMAKE_CURRENT_LIST_FILE}  PATH)

  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} ${_cmake_module_path}/FindPyCups.py OUTPUT_VARIABLE pycups)
  IF(pycups)
    SET(PYCUPS_FOUND TRUE)
  ENDIF(pycups)

  IF(PYCUPS_FOUND)
    IF(NOT PYCUPS_FIND_QUIETLY)
      MESSAGE(STATUS "Found PyCups")
    ENDIF(NOT PYCUPS_FIND_QUIETLY)
  ELSE(PYCUPS_FOUND)
    IF(PYCUPS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PyCups")
    ENDIF(PYCUPS_FIND_REQUIRED)
  ENDIF(PYCUPS_FOUND)

ENDIF(PYCUPS_FOUND)
