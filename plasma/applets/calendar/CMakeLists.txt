project(plasma-applet-calendar)

set(calendar_SRCS
    calendar.cpp)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(../../libs)

kde4_add_plugin(plasma_applet_calendar ${calendar_SRCS})
target_link_libraries(plasma_applet_calendar ${PLASMA_LIBS} plasmaclock ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_calendar DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR})
