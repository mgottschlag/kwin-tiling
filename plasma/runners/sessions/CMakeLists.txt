set(krunner_sessions_SRCS
    sessionrunner.cpp
)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(krunner_sessions_SRCS ${screensaver_xml} screensaver_interface)

kde4_add_plugin(krunner_sessions ${krunner_sessions_SRCS})
target_link_libraries(krunner_sessions ${KDE4_KIO_LIBS} plasma kworkspace)

install(TARGETS krunner_sessions DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-sessions.desktop DESTINATION ${SERVICES_INSTALL_DIR})

