add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_SUPPORT)

########### next target ###############

set(kcm_accessibility_PART_SRCS accessibility.cpp )


kde4_add_ui3_files(kcm_accessibility_PART_SRCS accessibilityconfigwidget.ui )

kde4_add_plugin(kcm_accessibility ${kcm_accessibility_PART_SRCS})


target_link_libraries(kcm_accessibility  ${KDE4_KIO_LIBS} ${QT_QT3SUPPORT_LIBRARY}) 

install(TARGETS kcm_accessibility  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES accessibility.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

