
set(panel_SRCS
    panel.cpp)

include_directories(
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)

kde4_add_plugin(plasma_containment_panel ${panel_SRCS})
target_link_libraries(plasma_containment_panel ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} kephal)

install(TARGETS plasma_containment_panel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-panel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
