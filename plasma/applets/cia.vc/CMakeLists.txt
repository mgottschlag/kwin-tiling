project(plasma-ciavc)

set(ciavc_SRCS
    ciavc.cpp)

kde4_automoc(${ciavc_SRCS})
kde4_add_plugin(plasma_applet_ciavc ${ciavc_SRCS})
target_link_libraries(plasma_applet_ciavc plasma ${KDE4_KIO_LIBS})
set_target_properties(plasma_applet_ciavc PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS plasma_applet_ciavc DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-ciavc.desktop DESTINATION ${SERVICES_INSTALL_DIR})
