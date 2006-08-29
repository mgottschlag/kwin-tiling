
add_subdirectory( Templates ) 

install( FILES Home.desktop System.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kdesktop/DesktopLinks )
install( FILES directory.trash directory.autostart directory.templates directory.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kdesktop )
