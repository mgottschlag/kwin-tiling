
# set up
#cmake_minimum_required(VERSION 2.6)

project(plasma-python)
#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# compile SIP
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${SIP_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH}
    # Qt
    ${KDE4_INCLUDES} ${QT_INCLUDE_DIR} ${QT_QT_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTSQL_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR} ${QT_QTSVG_INCLUDE_DIR} ${QT_QTUITOOLS_INCLUDE_DIR}
    # ${QT_QTOPENGL_INCLUDE_DIR} 
    # KDE
    ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDE_DIR}/KDE ${KDE4_INCLUDE_DIR}/solid ${KDE4_INCLUDE_DIR}/kio ${KDE4_INCLUDE_DIR}/kdeprint ${KDE4_INCLUDE_DIR}/kdeprint/lpr ${KDE4_INCLUDE_DIR}/dom ${KDE4_INCLUDE_DIR}/ksettings
    # Plasma
    ${KDE4_INCLUDE_DIR}/plasma )#${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasma)

# XX move this into a CMakeLists.txt for sip/. Gets boggling with things 
# looking for print_status.cmake in wrong places etc.
set(SIP_INCLUDES ${PYQT4_SIP_DIR} ${PYKDE4_SIP_DIR} sip)
set(SIP_CONCAT_PARTS 8)
set(SIP_TAGS ALL WS_X11 ${PYQT4_VERSION_TAG} )#${PYKDE4_VERSION_TAG})
set(SIP_EXTRA_OPTIONS -g)
add_sip_python_module(PyKDE4.plasma sip/plasmamod.sip
    plasma
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTUITOOLS_LIBRARY}
    ${KDE4_KIO_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
)

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
install(FILES plasma.py DESTINATION ${PYTHON_SITE_PACKAGES_DIR})

###########################################################################
# Test plasmoids

# install the test plugin
#install(FILES test/plasma-hello-python-world.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(DIRECTORY test/plasma-hello-python-world DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids)

install(FILES test/plasma-pyclock.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(DIRECTORY test/plasma-pyclock DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids)

#install(FILES test/plasma-nestclock.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(DIRECTORY test/plasma-nestclock DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids)

install(FILES test/plasma-dataengine-pytime.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(DIRECTORY test/plasma-dataengine-pytime DESTINATION ${DATA_INSTALL_DIR}/plasma/engines)
