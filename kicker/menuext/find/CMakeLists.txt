


########### next target ###############

set(kickermenu_find_PART_SRCS findmenu.cpp )

kde4_automoc(${kickermenu_find_PART_SRCS})

kde4_add_plugin(kickermenu_find ${kickermenu_find_PART_SRCS})


target_link_libraries(kickermenu_find  ${KDE4_KDEUI_LIBS} kickermain kworkspace )

install(TARGETS kickermenu_find  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES find.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )
install( FILES kfind.desktop websearch.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext/find )

