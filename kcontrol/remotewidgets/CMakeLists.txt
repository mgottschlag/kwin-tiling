########### next target ###############

set(kcm_remotewidgets_PART_SRCS remotewidgets.cpp )
kde4_add_ui_files(kcm_remotewidgets_PART_SRCS generalpage.ui rulesdialog.ui)

kde4_add_plugin(kcm_remotewidgets ${kcm_remotewidgets_PART_SRCS})

target_link_libraries(kcm_remotewidgets ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_remotewidgets  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES remotewidgets.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
