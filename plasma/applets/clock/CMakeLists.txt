
set(clock_SRCS
    clock.cpp)

kde4_add_ui_files(clock_SRCS clockConfig.ui )
kde4_add_plugin(plasma_applet_clock ${clock_SRCS})
target_link_libraries(plasma_applet_clock plasma ${KDE4_KIO_LIBS})
set_target_properties(plasma_applet_clock PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS plasma_applet_clock DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-clock-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
