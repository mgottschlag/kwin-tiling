project(plasma-python)

include(PythonMacros)

# install the library, .desktop, and plasma.py
python_install(pyappletscript.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
python_install(plasma_importer.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
python_install(pydataengine.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
python_install(pyrunner.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
python_install(pywallpaper.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)

# remove the following three lines once we branch 4.5, they are only necessary as long
# as people update kdebase but not yet kdelibs (PYTHON_SITE_PACKAGES_INSTALL_DIR has just
# recently been added to FindPYthonLib.cmake and should be used as install destination). Alex
if(NOT PYTHON_SITE_PACKAGES_INSTALL_DIR)
  set(PYTHON_SITE_PACKAGES_INSTALL_DIR ${PYTHON_SITE_PACKAGES_DIR} )
endif(NOT PYTHON_SITE_PACKAGES_INSTALL_DIR)

python_install(plasmascript.py ${PYTHON_SITE_PACKAGES_INSTALL_DIR}/PyKDE4)

install(FILES plasma-scriptengine-applet-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-dataengine-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-runner-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-wallpaper-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

###########################################################################
# Test plasmoids
option(INSTALL_PYTHON_TEST_PLASMA "Install test python applet" OFF)

if(INSTALL_PYTHON_TEST_PLASMA )

 install(DIRECTORY test/plasma-applet-pyclock DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids)

 #TODO where install it ?
 #install(DIRECTORY test/plasma-dataengine-pytime DESTINATION ${DATA_INSTALL_DIR}/plasma/engines)

endif(INSTALL_PYTHON_TEST_PLASMA)
