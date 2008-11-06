project(plasma-python)

# install the library, .desktop, and plasma.py
install(FILES pyappletscript.py DESTINATION ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
install(FILES plasma_importer.py DESTINATION ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
install(FILES package_pythoid.py DESTINATION ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
install(FILES pydataengine.py DESTINATION ${DATA_INSTALL_DIR}/plasma_scriptengine_python)

install(FILES plasma-scriptengine-applet-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-packagestructure-python.desktop
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
