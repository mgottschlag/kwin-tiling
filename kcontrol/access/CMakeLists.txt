


########### next target ###############

set(kcm_access_PART_SRCS kcmaccess.cpp )


kde4_add_plugin(kcm_access ${kcm_access_PART_SRCS})


target_link_libraries(kcm_access  ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${X11_LIBRARIES})

install(TARGETS kcm_access  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kaccess_KDEINIT_SRCS kaccess.cpp main.cpp )


kde4_add_kdeinit_executable( kaccess ${kaccess_KDEINIT_SRCS})

target_link_libraries(kdeinit_kaccess  ${KDE4_KDEUI_LIBS} ${KDE4_PHONON_LIBS} ${X11_LIBRARIES})

install(TARGETS kdeinit_kaccess ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kaccess         ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kcmaccess.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kaccess.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kaccess.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/kaccess/ )
