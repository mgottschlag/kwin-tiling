


########### next target ###############

set(kickermenu_kdeprint_PART_SRCS print_mnu.cpp )

add_definitions(-D_KDEPRINT_COMPILE )

kde4_automoc(${kickermenu_kdeprint_PART_SRCS})

kde4_add_plugin(kickermenu_kdeprint ${kickermenu_kdeprint_PART_SRCS})


target_link_libraries(kickermenu_kdeprint  ${KDE4_KIO_LIBS} ${KDE4_KDEPRINT_LIBS} kickermain )

install(TARGETS kickermenu_kdeprint  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES printmenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )

