
set(clock_SRCS
    clock.cpp)

kde4_add_ui_files(clock_SRCS clockConfig.ui )
kde4_add_plugin(plasma_applet_clock ${clock_SRCS})
include_directories( ../libplasmaclock/ ${CMAKE_CURRENT_BINARY_DIR}/../libplasmaclock/)
target_link_libraries(plasma_applet_clock plasma plasmaclock ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_clock DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-analogclock.desktop DESTINATION ${SERVICES_INSTALL_DIR})
