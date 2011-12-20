set(plasma_add_shortcut_to_menu_SRCS plasma-add-shortcut-to-menu.cpp)
kde4_add_executable(plasma-add-shortcut-to-menu ${plasma_add_shortcut_to_menu_SRCS})
target_link_libraries(plasma-add-shortcut-to-menu ${KDE4_KDECORE_LIBS})
install(TARGETS plasma-add-shortcut-to-menu DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)

install(FILES plasma-add-shortcut-to-menu.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

set(plasma_to_plasma_desktop_SRCS plasma-to-plasma-desktop.cpp)
kde4_add_executable(plasma-to-plasma-desktop ${plasma_to_plasma_desktop_SRCS})
target_link_libraries(plasma-to-plasma-desktop ${KDE4_KIO_LIBS})
install(TARGETS plasma-to-plasma-desktop DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)

install(FILES plasmarc-to-plasmadesktoprc.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(FILES plasma-to-plasmadesktop-shortcuts.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

install(FILES systray-to-notifications-widget.js DESTINATION ${DATA_INSTALL_DIR}/plasma-desktop/updates/ )
install(FILES addShowActivitiesManagerPlasmoid.js DESTINATION ${DATA_INSTALL_DIR}/plasma-desktop/updates/ )
