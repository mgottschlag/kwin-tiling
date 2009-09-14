project(plasma-containmentactions-zoom)

set(zoom_SRCS
    zoom.cpp
)

kde4_add_plugin(plasma_containmentactions_zoom ${zoom_SRCS})
target_link_libraries(plasma_containmentactions_zoom ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containmentactions_zoom DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-zoom.desktop DESTINATION ${SERVICES_INSTALL_DIR})
