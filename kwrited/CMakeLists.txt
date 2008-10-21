
set(kded_kwrited_PART_SRCS kwrited.cpp )
kde4_add_plugin(kded_kwrited ${kded_kwrited_PART_SRCS})
target_link_libraries(kded_kwrited  ${KDE4_KDEUI_LIBS} ${KDE4_KPTY_LIBS})
install(TARGETS kded_kwrited  DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES kwrited.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
install(FILES kwrited.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kwrited)
