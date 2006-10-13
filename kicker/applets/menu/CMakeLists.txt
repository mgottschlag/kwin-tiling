


########### next target ###############

set(menu_panelapplet_PART_SRCS menuapplet.cpp )

kde4_automoc(menu_panelapplet ${menu_panelapplet_PART_SRCS})


kde4_add_plugin(menu_panelapplet ${menu_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} menu_panelapplet )

target_link_libraries(menu_panelapplet  ${KDE4_KDEUI_LIBS} kickermain )

install(TARGETS menu_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES menuapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

