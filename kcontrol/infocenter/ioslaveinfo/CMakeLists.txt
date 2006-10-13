


########### next target ###############

set(kcm_ioslaveinfo_PART_SRCS kcmioslaveinfo.cpp )

kde4_automoc(kcm_ioslaveinfo ${kcm_ioslaveinfo_PART_SRCS})

kde4_add_plugin(kcm_ioslaveinfo ${kcm_ioslaveinfo_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_ioslaveinfo )

target_link_libraries(kcm_ioslaveinfo  ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_ioslaveinfo  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ioslaveinfo.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

