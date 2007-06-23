project(plasma-url)

set(url_SRCS
    url.cpp)

kde4_automoc(${url_SRCS})
kde4_add_plugin(plasma_applet_url ${url_SRCS})
target_link_libraries(plasma_applet_url plasma ${KDE4_KIO_LIBS})
set_target_properties(plasma_applet_url PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS plasma_applet_url DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-url.desktop DESTINATION ${SERVICES_INSTALL_DIR})
