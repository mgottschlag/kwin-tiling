include_directories( ${CMAKE_SOURCE_DIR}/workspace/plasma/lib/ ${CMAKE_SOURCE_DIR}/libkonq   )


########### next target ###############

set(sidebar_panelextension_PART_SRCS sidebarextension.cpp )

kde4_automoc(${sidebar_panelextension_PART_SRCS})

kde4_add_plugin(sidebar_panelextension ${sidebar_panelextension_PART_SRCS})


target_link_libraries(sidebar_panelextension  ${KDE4_KDECORE_LIBS} kickermain ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS} konq )

install(TARGETS sidebar_panelextension  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES sidebarextension.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/extensions )

