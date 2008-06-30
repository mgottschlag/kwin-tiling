project(plasma-applet-trash)

set(trash_SRCS
    trash.cpp)

kde4_add_plugin(plasma_applet_trash ${trash_SRCS})
target_link_libraries(plasma_applet_trash plasma ${KDE4_SOLID_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_trash DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-trash.desktop DESTINATION ${SERVICES_INSTALL_DIR})
