########### next target ###############

set(kcm_autostart_PART_SRCS 
   autostart.cpp )

kde4_automoc(${kcm_autostart_PART_SRCS})

kde4_add_ui_files(kcm_autostart_PART_SRCS autostartconfig.ui )

kde4_add_plugin(kcm_autostart ${kcm_autostart_PART_SRCS})

target_link_libraries(kcm_autostart  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kcm_autostart  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES autostart.desktop   DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )
