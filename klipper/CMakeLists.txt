
project(klipper)

include_directories(
   ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker/ )

########### next target ###############


########### next target ###############

set(libklipper_common_SRCS main.cpp toplevel.cpp urlgrabber.cpp configdialog.cpp clipboardpoll.cpp history.cpp historyitem.cpp historystringitem.cpp klipperpopup.cpp popupproxy.cpp  historyimageitem.cpp historyurlitem.cpp )


set(klipper_KDEINIT_SRCS ${libklipper_common_SRCS})

kde4_automoc(${klipper_KDEINIT_SRCS})

kde4_add_kdeinit_executable( klipper ${klipper_KDEINIT_SRCS})

target_link_libraries(kdeinit_klipper  ${KDE4_KDE3SUPPORT_LIBS}  )
if (X11_Xfixes_FOUND)
  target_link_libraries(kdeinit_klipper ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)

install(TARGETS kdeinit_klipper  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( klipper kdeinit_klipper )
install(TARGETS klipper DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(klipper_panelapplet_PART_SRCS applet.cpp ${libklipper_common_SRCS})

kde4_automoc(${klipper_panelapplet_PART_SRCS})


kde4_add_plugin(klipper_panelapplet ${klipper_panelapplet_PART_SRCS})


target_link_libraries(klipper_panelapplet  ${KDE4_KDE3SUPPORT_LIBS} kworkspace kickermain  )
if (X11_Xfixes_FOUND)
	target_link_libraries(klipper_panelapplet  ${X11_Xfixes_LIB} )
endif (X11_Xfixes_FOUND)

install(TARGETS klipper_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES klipper.desktop  DESTINATION  ${XDG_APPS_DIR})
install(FILES klipper.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES klipper.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets)
install(FILES klipperrc.upd klippershortcuts.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS klipper-1-2.pl klipper-kde31.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})


kde4_install_icons( ${ICON_INSTALL_DIR}  )

