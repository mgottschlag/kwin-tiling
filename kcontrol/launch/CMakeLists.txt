


########### next target ###############

set(kcm_launch_PART_SRCS kcmlaunch.cpp )

set(kdesktop_xml  ${CMAKE_SOURCE_DIR}/workspace/kdesktop/org.kde.kdesktop.Desktop.xml)
QT4_ADD_DBUS_INTERFACE( kcm_launch_PART_SRCS ${kdesktop_xml} kdesktop_interface )


kde4_automoc(${kcm_launch_PART_SRCS})

kde4_add_plugin(kcm_launch ${kcm_launch_PART_SRCS})


target_link_libraries(kcm_launch  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_launch  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmlaunch.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

