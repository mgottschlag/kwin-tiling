project(plasma-searchbox)


set(searchbox_SRCS searchbox.cpp)


kde4_add_plugin(plasma_applet_searchbox ${searchbox_SRCS})
target_link_libraries(plasma_applet_searchbox 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_searchbox
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-searchbox.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
