


########### next target ###############

set(kickermenu_remotemenu_PART_SRCS remotemenu.cpp )

kde4_automoc(${kickermenu_remotemenu_PART_SRCS})


kde4_add_plugin(kickermenu_remotemenu ${kickermenu_remotemenu_PART_SRCS})


target_link_libraries(kickermenu_remotemenu  ${KDE4_KDEUI_LIBS} kickermain )

install(TARGETS kickermenu_remotemenu  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES remotemenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )

