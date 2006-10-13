


########### next target ###############

set(kcm_kded_PART_SRCS kcmkded.cpp )

kde4_automoc(kcm_kded ${kcm_kded_PART_SRCS})

kde4_add_plugin(kcm_kded ${kcm_kded_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kded )

target_link_libraries(kcm_kded  ${KDE4_KIO_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY} )

install(TARGETS kcm_kded  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmkded.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
