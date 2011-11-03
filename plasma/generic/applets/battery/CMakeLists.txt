project(plasma-battery)

set(battery_SRCS
    battery.cpp)

kde4_add_ui_files(battery_SRCS batteryConfig.ui )
kde4_add_plugin(plasma_applet_battery ${battery_SRCS})
target_link_libraries(plasma_applet_battery ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS plasma_applet_battery DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-battery-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
    battery-oxygen.svgz
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

