project(batterymonitor)

install(DIRECTORY contents
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.batterymonitor/)
install(FILES metadata.desktop
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.batterymonitor)

install(FILES metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-batterymonitor.desktop)
