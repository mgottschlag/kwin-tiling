


########### next target ###############

set(kcm_taskbar_PART_SRCS kcmtaskbar.cpp )

kde4_automoc(${kcm_taskbar_PART_SRCS})

kde4_add_ui_files(kcm_taskbar_PART_SRCS kcmtaskbarui.ui )

kde4_add_plugin(kcm_taskbar ${kcm_taskbar_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_taskbar )

target_link_libraries(kcm_taskbar  taskbar ${KDE4_KDEUI_LIBS} )

install(TARGETS kcm_taskbar  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmtaskbar.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

