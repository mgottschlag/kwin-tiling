


########### next target ###############

set(kickermenu_konsole_PART_SRCS konsole_mnu.cpp konsolebookmarkhandler.cpp )

kde4_automoc(${kickermenu_konsole_PART_SRCS})

kde4_add_plugin(kickermenu_konsole ${kickermenu_konsole_PART_SRCS})


target_link_libraries(kickermenu_konsole  ${KDE4_KDE3SUPPORT_LIBS} kworkspace kickermain  ${QT_QTXML_LIBRARY} )

install(TARGETS kickermenu_konsole  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES konsolemenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )

