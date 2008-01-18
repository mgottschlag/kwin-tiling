

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kcm_kthememanager_PART_SRCS knewthemedlg.cpp ktheme.cpp kthememanager.cpp )

set(kwin_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/kwin/org.kde.KWin.xml)
QT4_ADD_DBUS_INTERFACE( kcm_kthememanager_PART_SRCS ${kwin_xml} kwin_interface )

set(plasma_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/plasma/org.kde.plasma.App.xml)
QT4_ADD_DBUS_INTERFACE( kcm_kthememanager_PART_SRCS ${plasma_xml} plasma_interface )



kde4_add_ui_files(kcm_kthememanager_PART_SRCS newthemewidget.ui)
kde4_add_ui3_files(kcm_kthememanager_PART_SRCS kthemedlg.ui )

kde4_add_plugin(kcm_kthememanager ${kcm_kthememanager_PART_SRCS})


target_link_libraries(kcm_kthememanager   ${KDE4_KDE3SUPPORT_LIBS}  ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTXML_LIBRARY})

install(TARGETS kcm_kthememanager  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kthememanager.desktop installktheme.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(PROGRAMS kdeinstallktheme DESTINATION ${BIN_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}  )
