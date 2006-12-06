


########### next target ###############

set(lockout_panelapplet_PART_SRCS lockout.cpp )

kde4_automoc(${lockout_panelapplet_PART_SRCS})

kde4_add_plugin(lockout_panelapplet ${lockout_panelapplet_PART_SRCS})


target_link_libraries(lockout_panelapplet  ${KDE4_KDECORE_LIBS} kickermain )

install(TARGETS lockout_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES lockout.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )


