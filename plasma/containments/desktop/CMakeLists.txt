set(desktop_SRCS
    desktop.cpp)

set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.kde.krunner.Interface.xml)
QT4_ADD_DBUS_INTERFACE(desktop_SRCS ${krunner_xml} krunner_interface)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(desktop_SRCS ${screensaver_xml} screensaver_interface)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(desktop_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_plugin(plasma_containment_desktop ${desktop_SRCS})
target_link_libraries(plasma_containment_desktop plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_containment_desktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-desktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma_desktop_containment.kwinrules DESTINATION ${DATA_INSTALL_DIR}/kwin/default_rules)
