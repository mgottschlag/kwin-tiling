


########### next target ###############

set(run_panelapplet_PART_SRCS runapplet.cpp )

kde4_automoc(${run_panelapplet_PART_SRCS})

kde4_add_plugin(run_panelapplet ${run_panelapplet_PART_SRCS})


target_link_libraries(run_panelapplet  kworkspace ${KDE4_KDEUI_LIBS} kickermain )

install(TARGETS run_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES runapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

