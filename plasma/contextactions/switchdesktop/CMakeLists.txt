project(plasma-containmentactions-switchdesktop)

set(switchdesktop_SRCS
    desktop.cpp
)

kde4_add_plugin(plasma_containmentactions_switchdesktop ${switchdesktop_SRCS})
target_link_libraries(plasma_containmentactions_switchdesktop ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containmentactions_switchdesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-switchdesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
