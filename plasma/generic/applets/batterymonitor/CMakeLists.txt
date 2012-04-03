project(batterymonitor)

install(DIRECTORY contents
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/battery)
install(FILES metadata.desktop
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/battery)

install(FILES metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-batterymonitor.desktop)

install(FILES
        battery-oxygen.svgz
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
