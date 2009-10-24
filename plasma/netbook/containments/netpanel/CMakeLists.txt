
set(panel_SRCS
    panel.cpp
    dummytoolbox.cpp
    ../common/linearappletoverlay.cpp
    ../common/appletmovespacer.cpp
    )

kde4_add_plugin(plasma_containment_netpanel ${panel_SRCS})
target_link_libraries(plasma_containment_netpanel ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containment_netpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-netpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
