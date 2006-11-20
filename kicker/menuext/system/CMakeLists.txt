


########### next target ###############

set(kickermenu_systemmenu_PART_SRCS systemmenu.cpp )

kde4_automoc(${kickermenu_systemmenu_PART_SRCS})

kde4_add_plugin(kickermenu_systemmenu ${kickermenu_systemmenu_PART_SRCS})


target_link_libraries(kickermenu_systemmenu  ${KDE4_KIO_LIBS} kickermain )

install(TARGETS kickermenu_systemmenu  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES systemmenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )

