FILE(GLOB desktopfile *.desktop)

INSTALL( FILES ${desktopfile} DESTINATION ${DATA_INSTALL_DIR}/solid/devices )
