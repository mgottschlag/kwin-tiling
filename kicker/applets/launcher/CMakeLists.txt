include_directories( ${CMAKE_SOURCE_DIR}/libkonq ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core ${CMAKE_SOURCE_DIR}/kicker/kicker/ui  ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui ${CMAKE_BINARY_DIR}/workspace/kicker/kicker  )


########### next target ###############

#### ****** So many sources? This is recompiling all of kicker! This needs to be fixed! ****** ####
set(launcher_panelapplet_PART_SRCS  ${libkicker_core_SRCS} ${libkicker_buttons_SRCS} ${libkicker_ui_SRCS}
   quicklauncher.cpp 
   quickbutton.cpp 
   quickaddappsmenu.cpp 
   flowgridmanager.cpp 
   popularity.cpp 
   configdlg.cpp )

qt4_add_dbus_interface(launcher_panelapplet_PART_SRCS ../../../krunner/org.freedesktop.ScreenSaver.xml screensaver_interface )
qt4_add_dbus_interface(launcher_panelapplet_PART_SRCS ../../../kdesktop/org.kde.kdesktop.Desktop.xml kdesktop_interface )
qt4_add_dbus_interface(launcher_panelapplet_PART_SRCS ../../../krunner/org.kde.krunner.Interface.xml krunner_interface )

kde4_add_ui3_files(launcher_panelapplet_PART_SRCS ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/appletview.ui ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/nonKDEButtonSettings.ui)




kde4_automoc(${launcher_panelapplet_PART_SRCS})

kde4_add_ui3_files(launcher_panelapplet_PART_SRCS configdlgbase.ui )


kde4_add_kcfg_files(launcher_panelapplet_PART_SRCS prefs.kcfgc )

kde4_add_plugin(launcher_panelapplet ${launcher_panelapplet_PART_SRCS})


target_link_libraries(launcher_panelapplet ${KDE4_KDE3SUPPORT_LIBS} kworkspace konq kickermain )

install(TARGETS launcher_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES launcherapplet.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES quicklauncher.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

