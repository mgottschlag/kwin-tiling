


########### next target ###############

set(kickermenu_konqueror_PART_SRCS konqy_menu.cpp )

kde4_automoc(${kickermenu_konqueror_PART_SRCS})

kde4_add_plugin(kickermenu_konqueror ${kickermenu_konqueror_PART_SRCS})


target_link_libraries(kickermenu_konqueror  ${KDE4_KIO_LIBS} kickermain )

install(TARGETS kickermenu_konqueror  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES konquerormenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )

