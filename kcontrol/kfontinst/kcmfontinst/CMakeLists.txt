include_directories( 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/ 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/viewpart/ 
	 )


########### next target ###############

set(kcm_fontinst_PART_SRCS ${libkfontinstprint_SRCS}
   KCmFontInst.cpp 
   KFileFontIconView.cpp 
   KFileFontView.cpp 
   PrintDialog.cpp 
   SettingsDialog.cpp )

kde4_automoc(kcm_fontinst ${kcm_fontinst_PART_SRCS})

kde4_add_plugin(kcm_fontinst ${kcm_fontinst_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_fontinst )

target_link_libraries(kcm_fontinst  ${KDE4_KIO_LIBS} kfontinst ${KDE4_KDEPRINT_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kcm_fontinst  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmfontinst.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )


