set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules )

MACRO(PYKDE4_ADD_EXECUTABLE _pyname _exename)
    INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -DTARGET=${DATA_INSTALL_DIR}/printer-applet/${_pyname} -DLINK_NAME=${BIN_INSTALL_DIR}/${_exename} -P ${CMAKE_SOURCE_DIR}/workspace/printer-applet/cmake-modules/create_exe_symlink.cmake)" )
ENDMACRO(PYKDE4_ADD_EXECUTABLE)

macro_optional_find_package(PythonLibrary)
macro_optional_find_package(SIP)

find_package(PyQt4)
IF(NOT PYQT4_FOUND)
    macro_log_feature(PYQT4_FOUND "PyQt4" "PyQt4 was not found.  It is needed by printer-applet to run." "http://www.riverbankcomputing.co.uk/pyqt/" FALSE)
ENDIF(NOT PYQT4_FOUND)

find_package(PyKDE)
IF(NOT PYKDE_FOUND)
    macro_log_feature(PYKDE_FOUND "PyKDE" "PyKDE was not found.  It is needed by printer-applet to run." "http://www.riverbankcomputing.co.uk/pykde/" FALSE)
ENDIF(NOT PYKDE_FOUND)

find_package(PyCups)
IF(NOT PYQT4_FOUND)
    macro_log_feature(PYCUPS_FOUND "PyCups" "PyCups was not found.  It is needed by printer-applet to run." "http://cyberelk.net/tim/software/pycups/" FALSE)
ENDIF(NOT PYQT4_FOUND)

IF(PYQT4_FOUND AND PYKDE_FOUND AND PYCUPS_FOUND)
    install( FILES
        printer-applet-printers.ui
        printer-applet.py
        printer-applet.ui
        printer-appletui.rc
        DESTINATION ${DATA_INSTALL_DIR}/printer-applet )
    PYKDE4_ADD_EXECUTABLE(printer-applet.py printer-applet)
ENDIF(PYQT4_FOUND AND PYKDE_FOUND AND PYCUPS_FOUND)

find_package(SystemConfigPrinter)
IF(NOT SYSTEMCONFIGPRINTER_FOUND)
    macro_log_feature(SYSTEMCONFIGPRINTER_FOUND "system-config-printer" "system-config-printer was not found.  Some of its modules (cupshelpers.py and ppds.py) are optionally used by printer-applet for auto configure of new printers.  You should also install hal-cups-utils <svn co http://svn.fedorahosted.org/svn/hal-cups-utils/trunk/>" "http://cyberelk.net/tim/software/system-config-printer/" FALSE)
ENDIF(NOT SYSTEMCONFIGPRINTER_FOUND)
