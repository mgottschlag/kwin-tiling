
set(url_SRCS
    url.cpp)

kde4_add_plugin(plasma_applet_url ${url_SRCS})
target_link_libraries(plasma_applet_url plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_url DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-url.desktop DESTINATION ${SERVICES_INSTALL_DIR})
