add_subdirectory( sounds )


########### next target ###############

set(kcm_knotify_PART_SRCS knotify.cpp )

kde4_automoc(${kcm_knotify_PART_SRCS})

kde4_add_ui_files(kcm_knotify_PART_SRCS playersettings.ui )

kde4_add_plugin(kcm_knotify ${kcm_knotify_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_knotify )

target_link_libraries(kcm_knotify  ${KDE4_KIO_LIBS} knotifyconfig ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_knotify  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmnotify.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

install( FILES play.png DESTINATION  ${DATA_INSTALL_DIR}/kcontrol/pics)
