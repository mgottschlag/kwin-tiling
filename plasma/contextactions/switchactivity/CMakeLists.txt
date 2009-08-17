project(plasma-containmentactions-switchactivity)

set(switchactivity_SRCS
    switch.cpp
)

kde4_add_plugin(plasma_containmentactions_switchactivity ${switchactivity_SRCS})
target_link_libraries(plasma_containmentactions_switchactivity ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containmentactions_switchactivity DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-switchactivity.desktop DESTINATION ${SERVICES_INSTALL_DIR})
