


########### next target ###############

set(naughty_panelapplet_PART_SRCS NaughtyProcessMonitor.cpp NaughtyConfigDialog.cpp NaughtyApplet.cpp )

kde4_automoc(${naughty_panelapplet_PART_SRCS})

kde4_add_plugin(naughty_panelapplet ${naughty_panelapplet_PART_SRCS})


target_link_libraries(naughty_panelapplet  ${KDE4_KDEUI_LIBS} kickermain )

install(TARGETS naughty_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES naughtyapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )
install( FILES naughty-happy.png naughty-sad.png  DESTINATION  ${DATA_INSTALL_DIR}/naughtyapplet/pics )

kde4_install_icons( ${ICON_INSTALL_DIR}  )


