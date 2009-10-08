project(plasma-devicenotifier)

set(devicenotifier_SRCS
    devicenotifier.cpp
    notifierdialog.cpp
    deviceitem.cpp)

kde4_add_ui_files(devicenotifier_SRCS configurationpage.ui)
kde4_add_plugin(plasma_applet_devicenotifier ${devicenotifier_SRCS})
target_link_libraries(plasma_applet_devicenotifier ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS plasma_applet_devicenotifier DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-devicenotifier.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES test-predicate-openinwindow.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions )
