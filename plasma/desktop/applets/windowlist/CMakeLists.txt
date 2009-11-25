project(plasma-windowlist)

set(windowlist_SRCS WindowList.cpp)

kde4_add_plugin(plasma_applet_windowlist ${windowlist_SRCS})
target_link_libraries(plasma_applet_windowlist
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      taskmanager
                      kworkspace
                      )

install(TARGETS plasma_applet_windowlist DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-windowlist.desktop DESTINATION ${SERVICES_INSTALL_DIR})
