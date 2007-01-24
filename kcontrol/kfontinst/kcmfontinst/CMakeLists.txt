include_directories( 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/ 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/viewpart/ 
	 )

set(kcm_fontinst_PART_SRCS ${libkfontinstview_SRCS} ${libkfontinstjobrunner_SRCS}
    FontList.cpp GroupList.cpp KCmFontInst.cpp PrintDialog.cpp FontGroups.cpp FontLister.cpp DuplicatesDialog.cpp )

kde4_automoc(${kcm_fontinst_PART_SRCS})

kde4_add_plugin(kcm_fontinst ${kcm_fontinst_PART_SRCS})

target_link_libraries(kcm_fontinst ${KDE4_KIO_LIBS} kfontinstui kfontinst )

install(TARGETS kcm_fontinst  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES fontinst.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES group.desktop  DESTINATION  ${MIME_INSTALL_DIR}/fonts )

kde4_install_icons( ${DATA_INSTALL_DIR}/kfontinst/icons )
