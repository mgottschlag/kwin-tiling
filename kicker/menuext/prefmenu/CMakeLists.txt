


########### next target ###############

set(kickermenu_prefmenu_PART_SRCS prefmenu.cpp )

kde4_automoc(${kickermenu_prefmenu_PART_SRCS})

kde4_add_plugin(kickermenu_prefmenu ${kickermenu_prefmenu_PART_SRCS})


target_link_libraries(kickermenu_prefmenu  ${KDE4_KDE3SUPPORT_LIBS} kickermain kworkspace )

install(TARGETS kickermenu_prefmenu  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES prefmenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )
