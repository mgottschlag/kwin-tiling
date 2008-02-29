########### next target ###############

set(kcm_keys_PART_SRCS
  kglobalshortcutseditor.cpp
  globalshortcuts.cpp
  select_scheme_dialog.cpp
  )

kde4_add_ui_files( kcm_keys_PART_SRCS 
    kglobalshortcutseditor.ui
    select_scheme_dialog.ui )

kde4_add_plugin(kcm_keys ${kcm_keys_PART_SRCS})


target_link_libraries(kcm_keys  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${X11_X11_LIB})

install(TARGETS kcm_keys  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES keys.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES
    kde3.kksrc
    kde4.kksrc
    mac4.kksrc
    test.kksrc
    unix3.kksrc
    win3.kksrc
    win4.kksrc
    wm3.kksrc
    DESTINATION  ${DATA_INSTALL_DIR}/kcmkeys )



install( FILES kaccel.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install(PROGRAMS convertShortcuts.pl  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

