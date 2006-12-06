


########### next target ###############

set(systemtray_panelapplet_PART_SRCS systemtrayapplet.cpp )

kde4_automoc(${systemtray_panelapplet_PART_SRCS})


kde4_add_plugin(systemtray_panelapplet ${systemtray_panelapplet_PART_SRCS})


target_link_libraries(systemtray_panelapplet  ${KDE4_KDECORE_LIBS} kickermain ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS systemtray_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES systemtrayapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

