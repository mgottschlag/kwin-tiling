add_definitions(-DQT3_SUPPORT)

########### next target ###############

set(kcm_smserver_PART_SRCS kcmsmserver.cpp smserverconfigimpl.cpp )

kde4_automoc(${kcm_smserver_PART_SRCS})

kde4_add_ui_files(kcm_smserver_PART_SRCS smserverconfigdlg.ui )

kde4_add_plugin(kcm_smserver ${kcm_smserver_PART_SRCS})


target_link_libraries(kcm_smserver  ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kcm_smserver  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmsmserver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

