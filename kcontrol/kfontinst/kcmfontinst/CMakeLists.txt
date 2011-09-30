include_directories( 
	${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/viewpart/
	 )

set(kcm_fontinst_PART_SRCS ${libkfontinstview_SRCS} ${libkfontinstjobrunner_SRCS} ${libkfontinstdbusiface_SRCS}
    DuplicatesDialog.cpp FontList.cpp GroupList.cpp KCmFontInst.cpp PrintDialog.cpp
    FontFilter.cpp FontFilterProxyStyle.cpp FcQuery.cpp PreviewList.cpp)
# qt4_add_dbus_interface(kcm_fontinst_PART_SRCS ../dbus/org.kde.fontinst.xml FontinstIface)

kde4_add_plugin(kcm_fontinst ${kcm_fontinst_PART_SRCS})

target_link_libraries(kcm_fontinst ${KDE4_KIO_LIBS} ${KDE4_KDESU_LIBS} ${KDE4_KNEWSTUFF3_LIBRARY} kfontinstui kfontinst ${X11_LIBRARIES})

install(TARGETS kcm_fontinst  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES fontinst.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kfontinst.knsrc DESTINATION  ${CONFIG_INSTALL_DIR} )

kde4_install_icons( ${DATA_INSTALL_DIR}/kfontinst/icons )
