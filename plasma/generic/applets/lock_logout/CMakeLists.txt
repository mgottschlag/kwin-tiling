project(plasma-lockout)

set(lockout_SRCS
    lockout.cpp)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/screenlocker/dbus/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(lockout_SRCS ${screensaver_xml} screensaver_interface)

set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
QT4_ADD_DBUS_INTERFACE(lockout_SRCS ${krunner_xml} krunner_interface)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(lockout_SRCS ${ksmserver_xml} ksmserver_interface)

if(NOT WIN32)
kde4_add_ui_files(lockout_SRCS lockoutConfig.ui)
endif(NOT WIN32)
kde4_add_plugin(plasma_applet_lockout ${lockout_SRCS})
target_link_libraries(plasma_applet_lockout ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} kworkspace)

install(TARGETS plasma_applet_lockout DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-lockout.desktop DESTINATION ${SERVICES_INSTALL_DIR})
