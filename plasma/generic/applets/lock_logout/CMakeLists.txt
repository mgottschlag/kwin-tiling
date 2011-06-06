project(lockout)

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.lockout)

install(FILES plasma-applet-lockout.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
