project(plasma-showActivityManager)

set(showActivityManager_SRCS
    showActivityManager.cpp)

kde4_add_plugin(plasma_applet_showActivityManager ${showActivityManager_SRCS})
target_link_libraries(plasma_applet_showActivityManager ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_applet_showActivityManager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-org.kde.showActivityManager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES activities.svgz DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

