


########### next target ###############

set(dockbar_panelextension_PART_SRCS dockcontainer.cpp dockbarextension.cpp )

kde4_automoc(${dockbar_panelextension_PART_SRCS})

kde4_add_plugin(dockbar_panelextension ${dockbar_panelextension_PART_SRCS})


target_link_libraries(dockbar_panelextension  kickermain ${KDE4_KDEUI_LIBS} )

install(TARGETS dockbar_panelextension  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES dockbarextension.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/extensions )

