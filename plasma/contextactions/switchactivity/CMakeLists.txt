project(plasma-contextaction-switchactivity)

set(switchactivity_SRCS
    switch.cpp
)

kde4_add_plugin(plasma_contextaction_switchactivity ${switchactivity_SRCS})
target_link_libraries(plasma_contextaction_switchactivity ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_contextaction_switchactivity DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-contextaction-switchactivity.desktop DESTINATION ${SERVICES_INSTALL_DIR})
