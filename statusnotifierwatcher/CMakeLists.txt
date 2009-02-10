project(systemtraydaemon)

set(kded_systemtraydaemon_SRCS systemtraydaemon.cpp )

QT4_ADD_DBUS_ADAPTOR(kded_systemtraydaemon_SRCS     org.kde.SystemTrayDaemon.xml
                     systemtraydaemon.h SystemTrayDaemon)

kde4_add_plugin(kded_systemtraydaemon ${kded_systemtraydaemon_SRCS})


target_link_libraries(kded_systemtraydaemon  ${KDE4_KIO_LIBS} )

install(TARGETS kded_systemtraydaemon  DESTINATION ${PLUGIN_INSTALL_DIR})


install( FILES systemtraydaemon.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)

install(FILES org.kde.SystemTrayDaemon.xml
        DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
