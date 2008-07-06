set(plasma_add_shortcut_to_menu_SRCS plasma-add-shortcut-to-menu.cpp)
kde4_add_executable(plasma-add-shortcut-to-menu ${plasma_add_shortcut_to_menu_SRCS})
target_link_libraries(plasma-add-shortcut-to-menu ${KDE4_KDECORE_LIBS})
install(TARGETS plasma-add-shortcut-to-menu DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)

install(FILES plasma-add-shortcut-to-menu.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

