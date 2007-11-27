project(plasma-systemtray)

set(systemtray_SRCS
    systemtray.cpp
    systemtraycontainer.cpp
    systemtraywidget.cpp)

kde4_add_plugin(plasma_applet_systemtray ${systemtray_SRCS})
target_link_libraries(plasma_applet_systemtray ${KDE4_KDEUI_LIBS} plasma)

install(TARGETS plasma_applet_systemtray DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-systemtray.desktop DESTINATION ${SERVICES_INSTALL_DIR})

