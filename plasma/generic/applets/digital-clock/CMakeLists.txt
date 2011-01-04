project(plasma-dig-clock)

set(clock_SRCS
    clock.cpp)

kde4_add_ui_files(clock_SRCS clockConfig.ui )
kde4_add_ui_files(clock_SRCS )
kde4_add_plugin(plasma_applet_dig_clock ${clock_SRCS})
target_link_libraries(plasma_applet_dig_clock
                      ${KDE4_PLASMA_LIBS}
                      plasmaclock
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_dig_clock DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-digitalclock.desktop DESTINATION ${SERVICES_INSTALL_DIR})

