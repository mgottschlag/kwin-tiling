project(plasma-lockout)

set(lockout_SRCS
    lockout.cpp)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(lockout_SRCS ${screensaver_xml} screensaver_interface)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(lockout_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_plugin(plasma_applet_lockout ${lockout_SRCS})
target_link_libraries(plasma_applet_lockout plasma kworkspace ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_lockout DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-lockout.desktop DESTINATION ${SERVICES_INSTALL_DIR})
