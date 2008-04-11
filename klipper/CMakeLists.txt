KDE4_NO_ENABLE_FINAL(klipper)

set(libklipper_common_SRCS
    klipper.cpp
    urlgrabber.cpp
    configdialog.cpp
    clipboardpoll.cpp
    history.cpp
    historyitem.cpp
    historystringitem.cpp
    klipperpopup.cpp
    popupproxy.cpp
    historyimageitem.cpp
    historyurlitem.cpp
)

set(klipper_KDEINIT_SRCS ${libklipper_common_SRCS} main.cpp tray.cpp)

kde4_add_kdeinit_executable(klipper ${klipper_KDEINIT_SRCS})

target_link_libraries(kdeinit_klipper ${KDE4_KDEUI_LIBS})
if (X11_Xfixes_FOUND)
  target_link_libraries(kdeinit_klipper ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)

install(TARGETS kdeinit_klipper ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS klipper         ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES klipper.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES klipper.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES klipperrc.desktop DESTINATION ${CONFIG_INSTALL_DIR} RENAME klipperrc)
