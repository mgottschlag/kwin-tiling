
set(panel_SRCS
    panel.cpp)

kde4_add_plugin(plasma_containment_midpanel ${panel_SRCS})
target_link_libraries(plasma_containment_midpanel ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containment_midpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-midpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
