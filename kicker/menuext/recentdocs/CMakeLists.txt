


########### next target ###############

set(kickermenu_recentdocs_PART_SRCS recentdocsmenu.cpp )

kde4_automoc(${kickermenu_recentdocs_PART_SRCS})

kde4_add_plugin(kickermenu_recentdocs ${kickermenu_recentdocs_PART_SRCS})


target_link_libraries(kickermenu_recentdocs  ${KDE4_KDEUI_LIBS} kickermain kworkspace ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kickermenu_recentdocs  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES recentdocs.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )


