
add_subdirectory( componentservices ) 



########### next target ###############

set(kcm_componentchooser_PART_SRCS componentchooser.cpp kcm_componentchooser.cpp )

kde4_automoc(${kcm_componentchooser_PART_SRCS})

kde4_add_ui_files(kcm_componentchooser_PART_SRCS browserconfig_ui.ui terminalemulatorconfig_ui.ui emailclientconfig_ui.ui componentchooser_ui.ui componentconfig_ui.ui) 

kde4_add_plugin(kcm_componentchooser ${kcm_componentchooser_PART_SRCS})


target_link_libraries(kcm_componentchooser  ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcm_componentchooser  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES componentchooser.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

