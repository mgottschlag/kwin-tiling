project(plasma-dig-clock)

include_directories( ../libplasmaclock/
${CMAKE_CURRENT_BINARY_DIR}/../libplasmaclock/)

set(clock_SRCS
    clock.cpp)

kde4_add_ui_files(clock_SRCS clockConfig.ui )
kde4_add_ui_files(clock_SRCS )
kde4_add_plugin(plasma_applet_dig_clock ${clock_SRCS})
target_link_libraries(plasma_applet_dig_clock plasma plasmaclock ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_dig_clock DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-digitalclock.desktop DESTINATION ${SERVICES_INSTALL_DIR})

