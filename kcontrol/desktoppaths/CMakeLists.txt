set(kcm_desktoppaths_PART_SRCS
   globalpaths.cpp
   kcmdesktoppaths.cpp
)

kde4_add_plugin(kcm_desktoppaths ${kcm_desktoppaths_PART_SRCS})

target_link_libraries(kcm_desktoppaths ${KDE4_KCMUTILS_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kcm_desktoppaths DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES desktoppath.desktop DESTINATION ${SERVICES_INSTALL_DIR})

