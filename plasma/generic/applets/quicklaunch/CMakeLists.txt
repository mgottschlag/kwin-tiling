project(plasma-quicklaunch)

set(quicklaunch_SRCS
  icongridlayout.cpp
  launcher.cpp
  launcherdata.cpp
  launchergrid.cpp
  popup.cpp
  popuplauncherlist.cpp
  quicklaunch.cpp
)

kde4_add_ui_files(quicklaunch_SRCS quicklaunchConfig.ui)
kde4_add_plugin(plasma_applet_quicklaunch ${quicklaunch_SRCS})

target_link_libraries(plasma_applet_quicklaunch
  ${KDE4_PLASMA_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBRARY})

install(TARGETS plasma_applet_quicklaunch DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-quicklaunch.desktop DESTINATION ${SERVICES_INSTALL_DIR})
