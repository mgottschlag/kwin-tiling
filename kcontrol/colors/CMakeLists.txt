


########### next target ###############

set(kcm_colors_PART_SRCS colorscm.cpp widgetcanvas.cpp ${libkrd_SRCS})

kde4_automoc(${kcm_colors_PART_SRCS})

kde4_add_plugin(kcm_colors ${kcm_colors_PART_SRCS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcm_colors)

target_link_libraries(kcm_colors ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDEFX_LIBS})

install(TARGETS kcm_colors  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES colors.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

