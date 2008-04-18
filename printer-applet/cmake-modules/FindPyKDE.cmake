# Find PyKde
# ~~~~~~~~~~
# Copyright (c) 2008, Jonathan Riddell <jriddell@ubuntu.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# PyKde website: http://www.riverbankcomputing.co.uk/pykde/index.php
#
# Find the installed version of PyKDE.

IF(PYKDE_FOUND)
  # Already in cache, be silent
  SET(PYKDE_FOUND TRUE)
ELSE(PYKDE_FOUND)

  GET_FILENAME_COMPONENT(_cmake_module_path ${CMAKE_CURRENT_LIST_FILE}  PATH)

  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} ${_cmake_module_path}/FindPyKDE.py OUTPUT_VARIABLE pykde)
  IF(pykde)
    SET(PYKDE_FOUND TRUE)
  ENDIF(pykde)

  IF(PYKDE_FOUND)
    IF(NOT PYKDE_FIND_QUIETLY)
      MESSAGE(STATUS "Found PyKde")
    ENDIF(NOT PYKDE_FIND_QUIETLY)
  ELSE(PYKDE_FOUND)
    IF(PYKDE_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PyKDE")
    ENDIF(PYKDE_FIND_REQUIRED)
  ENDIF(PYKDE_FOUND)

ENDIF(PYKDE_FOUND)
