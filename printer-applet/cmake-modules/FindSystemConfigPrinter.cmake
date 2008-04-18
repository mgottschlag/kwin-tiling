# Find PyCups
# ~~~~~~~~~~
# Copyright (c) 2008, Jonathan Riddell <jriddell@ubuntu.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# system-config-printer website: http://cyberelk.net/tim/software/system-config-printer/
#
# Find the installed version of System Config Printer

IF(SYSTEMCONFIGPRINTER_FOUND)
  # Already in cache, be silent
  SET(SYSTEMCONFIGPRINTER_FOUND TRUE)
ELSE(SYSTEMCONFIGPRINTER_FOUND)

  GET_FILENAME_COMPONENT(_cmake_module_path ${CMAKE_CURRENT_LIST_FILE}  PATH)

  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} ${_cmake_module_path}/FindSystemConfigPrinter.py OUTPUT_VARIABLE systemconfigprinter)
  IF(systemconfigprinter)
    SET(SYSTEMCONFIGPRINTER_FOUND TRUE)
  ENDIF(systemconfigprinter)

  IF(SYSTEMCONFIGPRINTER_FOUND)
    IF(NOT SYSTEMCONFIGPRINTER_FIND_QUIETLY)
      MESSAGE(STATUS "Found System Config Printer")
    ENDIF(NOT SYSTEMCONFIGPRINTER_FIND_QUIETLY)
  ELSE(SYSTEMCONFIGPRINTER_FOUND)
    IF(SYSTEMCONFIGPRINTER_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find system-config-printer")
    ENDIF(SYSTEMCONFIGPRINTER_FIND_REQUIRED)
  ENDIF(SYSTEMCONFIGPRINTER_FOUND)

ENDIF(SYSTEMCONFIGPRINTER_FOUND)
