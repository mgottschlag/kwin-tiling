project(devicenotifier)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.devicenotifier)

install(FILES plasma-applet-devicenotifier.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

#install(FILES test-predicate-openinwindow.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions )
