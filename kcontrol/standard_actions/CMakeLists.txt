set(kcm_standard_actions_PART_SRCS standard_actions_module.cpp)

# kde4_add_ui_files(kcm_standard_actions_PART_SRCS)

kde4_add_plugin(kcm_standard_actions ${kcm_standard_actions_PART_SRCS})

target_link_libraries(kcm_standard_actions ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${X11_X11_LIB})

install(TARGETS kcm_standard_actions DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES standard_actions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

