
set(panel_SRCS
    panel.cpp)

kde4_add_plugin(plasma_containment_panel ${panel_SRCS})
target_link_libraries(plasma_containment_panel plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_containment_panel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-panel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
