


########### next target ###############

set(kcm_kthememanager_PART_SRCS knewthemedlg.cpp ktheme.cpp kthememanager.cpp )

kde4_automoc(${kcm_kthememanager_PART_SRCS})

kde4_add_ui3_files(kcm_kthememanager_PART_SRCS kthemedlg.ui newthemewidget.ui )

kde4_add_plugin(kcm_kthememanager ${kcm_kthememanager_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kthememanager )

target_link_libraries(kcm_kthememanager  ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS}  ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTXML_LIBRARY})

install(TARGETS kcm_kthememanager  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES x-ktheme.desktop  DESTINATION  ${MIME_INSTALL_DIR}/application )
install( FILES kthememanager.desktop installktheme.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(PROGRAMS kdeinstallktheme DESTINATION ${BIN_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}  )
