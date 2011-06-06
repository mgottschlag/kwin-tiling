project(batterymonitor)

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.batterymonitor)

install(FILES plasma-applet-batterymonitor.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
