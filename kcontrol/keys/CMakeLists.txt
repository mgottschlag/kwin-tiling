

add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(kcm_keys_PART_SRCS 
   shortcuts.cpp 
   main.cpp 
   khotkeys.cpp 
   treeview.cpp )

kde4_automoc(${kcm_keys_PART_SRCS})

kde4_add_plugin(kcm_keys ${kcm_keys_PART_SRCS})


target_link_libraries(kcm_keys ${KDE4_KDE3SUPPORT_LIBS} ${X11_X11_LIB})

install(TARGETS kcm_keys  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES keys.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES wm3.kksrc kde3.kksrc kde4.kksrc win3.kksrc win4.kksrc mac4.kksrc unix3.kksrc  DESTINATION  ${DATA_INSTALL_DIR}/kcmkeys )
install( FILES kaccel.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install(PROGRAMS convertShortcuts.pl  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
kde4_install_icons( ${ICON_INSTALL_DIR}  )

