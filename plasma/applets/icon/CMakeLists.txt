
set(icon_SRCS
    icon.cpp)

kde4_add_plugin(plasma_applet_icon ${icon_SRCS})
target_link_libraries(plasma_applet_icon plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_icon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-icon.desktop DESTINATION ${SERVICES_INSTALL_DIR})
