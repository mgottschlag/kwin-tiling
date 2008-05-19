project(plasma-folderview)

set(folderview_SRCS
    folderview.cpp
    proxymodel.cpp)

kde4_add_ui_files(folderview_SRCS folderviewConfig.ui)
kde4_add_plugin(plasma_applet_folderview ${folderview_SRCS})
target_link_libraries(plasma_applet_folderview plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_folderview DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-folderview.desktop DESTINATION ${SERVICES_INSTALL_DIR})

