project(lockout)

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.lockout)

install(FILES
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-lockout.desktop)
