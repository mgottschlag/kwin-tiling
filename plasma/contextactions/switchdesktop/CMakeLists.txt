project(plasma-contextaction-switchdesktop)

set(switchdesktop_SRCS
    desktop.cpp
)

kde4_add_plugin(plasma_contextaction_switchdesktop ${switchdesktop_SRCS})
target_link_libraries(plasma_contextaction_switchdesktop ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_contextaction_switchdesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-contextaction-switchdesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
