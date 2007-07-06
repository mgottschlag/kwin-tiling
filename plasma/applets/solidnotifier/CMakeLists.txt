project(plasma-solidnotifier)

set(solidnotifier_SRCS
    solidnotifier.cpp)

kde4_automoc(${solidnotifier_SRCS})
kde4_add_ui_files(solidnotifier_SRCS )
kde4_add_plugin(plasma_applet_solidnotifier ${solidnotifier_SRCS})
target_link_libraries(plasma_applet_solidnotifier plasma ${KDE4_KIO_LIBS})
set_target_properties(plasma_applet_solidnotifier PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS plasma_applet_solidnotifier DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-solidnotifier-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
