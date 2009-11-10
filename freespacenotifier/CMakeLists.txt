add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)


########### next target ###############

set(kded_susefreespacenotifier_SRCS freespacenotifier.cpp module.cpp)

kde4_add_ui_files(kded_susefreespacenotifier_SRCS freespacewidget.ui)

kde4_add_plugin(kded_susefreespacenotifier ${kded_susefreespacenotifier_SRCS})

target_link_libraries(kded_susefreespacenotifier ${KDE4_KIO_LIBS} )

install(TARGETS kded_susefreespacenotifier  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES susefreespacenotifier.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
