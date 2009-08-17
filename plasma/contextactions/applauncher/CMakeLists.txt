project(plasma-contextaction-applauncher)

set(applauncher_SRCS
    launch.cpp
)

kde4_add_plugin(plasma_contextaction_applauncher ${applauncher_SRCS})
target_link_libraries(plasma_contextaction_applauncher ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_contextaction_applauncher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-contextaction-applauncher.desktop DESTINATION ${SERVICES_INSTALL_DIR})
