include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/shared/ )


########### install files ###############

set(khotkeys_update_SRCS update.cpp )

qt4_add_dbus_interface(khotkeys_update_SRCS ../app/org.kde.khotkeys.xml khotkeysiface)

kde4_add_executable( khotkeys_update ${khotkeys_update_SRCS})

target_link_libraries( khotkeys_update khotkeysprivate kworkspace ${KDE4_KDE3SUPPORT_LIBS} )
if(X11_XTest_FOUND)
  target_link_libraries( khotkeys_update ${X11_XTest_LIB} )
endif(X11_XTest_FOUND)

install(TARGETS khotkeys_update DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )
