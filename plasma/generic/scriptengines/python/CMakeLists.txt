project(plasma-python)

INCLUDE(PythonMacros)

# install the library, .desktop, and plasma.py
PYTHON_INSTALL(pyappletscript.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
PYTHON_INSTALL(plasma_importer.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
PYTHON_INSTALL(pydataengine.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
PYTHON_INSTALL(plasmascript.py ${PYTHON_SITE_PACKAGES_DIR}/PyKDE4)

install(FILES plasma-scriptengine-applet-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-dataengine-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

###########################################################################
# Test plasmoids
OPTION(INSTALL_PYTHON_TEST_PLASMA "Install test python applet" OFF)

if(INSTALL_PYTHON_TEST_PLASMA )

 install(DIRECTORY test/plasma-applet-pyclock DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/ PATTERN .svn EXCLUDE)

 #TODO where install it ?
 #install(DIRECTORY test/plasma-dataengine-pytime DESTINATION ${DATA_INSTALL_DIR}/plasma/engines)

endif(INSTALL_PYTHON_TEST_PLASMA)
