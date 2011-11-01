project(devicenotifier)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/notifier)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-devicenotifier.desktop)

install(FILES test-predicate-openinwindow.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions )
