

########### next target ###############

set(khotkeys_KDEINIT_SRCS app.cpp )


kde4_add_kdeinit_executable( khotkeys ${khotkeys_KDEINIT_SRCS})

target_link_libraries(kdeinit_khotkeys libkhotkeys kworkspace ${KDE4_KDE3SUPPORT_LIBS} )
if(X11_XTest_FOUND)
  target_link_libraries(kdeinit_khotkeys  ${X11_XTest_LIB}  )
endif(X11_XTest_FOUND)

install(TARGETS kdeinit_khotkeys  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( khotkeys kdeinit_khotkeys )
install(TARGETS khotkeys DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

  #set(kded_khotkeys_PART_SRCS kded.cpp )
  #
  #qt4_add_dbus_adaptor(kded_khotkeys_PART_SRCS org.kde.khotkeys.xml kded.h KHotKeysModule)
  #
  #kde4_add_plugin(kded_khotkeys ${kded_khotkeys_PART_SRCS})
  #install(FILES org.kde.khotkeys.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
  #
  #target_link_libraries(kded_khotkeys libkhotkeys kworkspace ${KDE4_KDE3SUPPORT_LIBS} )
  #if(X11_XTest_FOUND)
  #    target_link_libraries(kded_khotkeys ${X11_XTest_LIB} )
  #endif(X11_XTest_FOUND)
  #
  #install(TARGETS kded_khotkeys  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

  # install( FILES khotkeys.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )


