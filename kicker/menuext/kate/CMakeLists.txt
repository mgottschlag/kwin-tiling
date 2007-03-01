########### next target ###############

set(kickermenu_kate_PART_SRCS katesessionmenu.cpp )

kde4_automoc(${kickermenu_kate_PART_SRCS})

kde4_add_plugin(kickermenu_kate ${kickermenu_kate_PART_SRCS})


target_link_libraries(kickermenu_kate kworkspace kickermain )

install(TARGETS kickermenu_kate DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES katesessionmenu.desktop DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )
