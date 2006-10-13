


########### next target ###############

set(kcm_accessibility_PART_SRCS accessibility.cpp )

kde4_automoc(kcm_accessibility ${kcm_accessibility_PART_SRCS})

kde4_add_ui3_files(kcm_accessibility_PART_SRCS accessibilityconfigwidget.ui )

kde4_add_plugin(kcm_accessibility ${kcm_accessibility_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_accessibility )

target_link_libraries(kcm_accessibility  ${KDE4_KDE3SUPPORT_LIBS}) 

install(TARGETS kcm_accessibility  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES accessibility.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

