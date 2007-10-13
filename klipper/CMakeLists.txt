KDE4_NO_ENABLE_FINAL(klipper)
include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/kicker/libkicker/ )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(libklipper_common_SRCS klipper.cpp urlgrabber.cpp configdialog.cpp clipboardpoll.cpp history.cpp historyitem.cpp historystringitem.cpp klipperpopup.cpp popupproxy.cpp  historyimageitem.cpp historyurlitem.cpp )

set(klipper_KDEINIT_SRCS ${libklipper_common_SRCS} main.cpp tray.cpp)

kde4_add_kdeinit_executable( klipper ${klipper_KDEINIT_SRCS})

target_link_libraries(kdeinit_klipper  ${KDE4_KDE3SUPPORT_LIBS}  )
if (X11_Xfixes_FOUND)
  target_link_libraries(kdeinit_klipper ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)

install(TARGETS kdeinit_klipper  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( klipper kdeinit_klipper )
install(TARGETS klipper DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install(FILES klipper.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES klipper.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES klipperrc.upd klippershortcuts.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS klipper-1-2.pl klipper-kde31.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}  )
