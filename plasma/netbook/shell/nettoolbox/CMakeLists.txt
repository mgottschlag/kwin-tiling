project(plasma-nettoolbox)


set(nettoolbox_SRCS nettoolbox.cpp)


kde4_add_plugin(plasma_toolbox_nettoolbox ${nettoolbox_SRCS})
target_link_libraries(plasma_toolbox_nettoolbox 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_toolbox_nettoolbox
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-toolbox-nettoolbox.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
