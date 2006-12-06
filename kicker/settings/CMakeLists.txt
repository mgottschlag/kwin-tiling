


########### next target ###############

set(kcm_kicker_PART_SRCS 
   positiontab_impl.cpp 
   hidingtab_impl.cpp 
   lookandfeeltab_impl.cpp 
   menutab_impl.cpp 
   extensionInfo.cpp 
   main.cpp 
   advancedDialog.cpp 
   positionconfig.cpp 
   hidingconfig.cpp 
   menuconfig.cpp 
   lookandfeelconfig.cpp )

kde4_automoc(${kcm_kicker_PART_SRCS})

kde4_add_ui3_files(kcm_kicker_PART_SRCS 
   positiontab.ui 
   hidingtab.ui 
   lookandfeeltab.ui 
   menutab.ui 
   advancedOptions.ui )


kde4_add_plugin(kcm_kicker ${kcm_kicker_PART_SRCS})


target_link_libraries(kcm_kicker  kickermain bgnd ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kicker  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kicker_config.desktop                  kicker_config_arrangement.desktop kicker_config_hiding.desktop                  kicker_config_menus.desktop kicker_config_appearance.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES panel.desktop  DESTINATION  ${XDG_APPS_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

