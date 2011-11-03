project(plasma-applet-calendar)

set(calendar_SRCS
    calendar.cpp)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

kde4_add_plugin(plasma_applet_calendar ${calendar_SRCS})
target_link_libraries(plasma_applet_calendar ${KDE4_PLASMA_LIBS} plasmaclock ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_calendar DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES mini-calendar.svgz DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/calendar/)
