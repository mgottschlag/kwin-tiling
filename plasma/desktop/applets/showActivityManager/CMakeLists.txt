project(plasma-showActivityManager)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.showActivityManager)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-org.kde.showActivityManager.desktop)

install(FILES activities.svgz
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
