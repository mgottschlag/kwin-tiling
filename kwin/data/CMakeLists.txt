
set(kwin_xml  ${CMAKE_CURRENT_SOURCE_DIR}/../org.kde.KWin.xml)

########### next target ###############
set(kwin_update_default_rules_SRCS update_default_rules.cpp)


kde4_add_executable( kwin_update_default_rules ${kwin_update_default_rules_SRCS})

target_link_libraries( kwin_update_default_rules ${KDE4_KDECORE_LIBS} )

install(TARGETS kwin_update_default_rules DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/  )

########### next target ###############
set( kwin_update_tabbox_settings_SRCS update_tabbox_settings.cpp )

kde4_add_executable( kwin_update_tabbox_settings ${kwin_update_tabbox_settings_SRCS} )

target_link_libraries( kwin_update_tabbox_settings ${KDE4_KDECORE_LIBS} )

install( TARGETS kwin_update_tabbox_settings DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )
########### next target ###############
set( kwin_update_tabbox_qml_settings_SRCS update_tabbox_qml_settings.cpp )

kde4_add_executable( kwin_update_tabbox_qml_settings ${kwin_update_tabbox_qml_settings_SRCS} )

target_link_libraries( kwin_update_tabbox_qml_settings ${KDE4_KDECORE_LIBS} )

install( TARGETS kwin_update_tabbox_qml_settings DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )

########### next target ###############
set( kwin_update_settings_49_SRCS update_kwin_49.cpp update_kwin_49_main.cpp )

kde4_add_executable( kwin_update_settings_49 ${kwin_update_settings_49_SRCS} )

target_link_libraries( kwin_update_settings_49 ${KDE4_KDECORE_LIBS} )

install( TARGETS kwin_update_settings_49 DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )

########### install files ###############

install( FILES fsp_workarounds_1.kwinrules  DESTINATION  ${DATA_INSTALL_DIR}/kwin/default_rules )
install( FILES pop.wav  DESTINATION  ${SOUND_INSTALL_DIR} )
install( FILES kwin_fsp_workarounds_1.upd kwin_update_tabbox_settings.upd kwin_remove_effects.upd kwin_update_tabbox_qml_settings.upd kwin_remove_delay_focus.upd kwin_update_49.upd DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS kwin_remove_delay_focus.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

