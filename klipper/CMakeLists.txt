kde4_no_enable_final(klipper)

set(libklipper_common_SRCS
    klipper.cpp
    urlgrabber.cpp
    configdialog.cpp
    history.cpp
    historyitem.cpp
    historystringitem.cpp
    klipperpopup.cpp
    popupproxy.cpp
    historyimageitem.cpp
    historyurlitem.cpp
    actionstreewidget.cpp
    editactiondialog.cpp
    clipcommandprocess.cpp
)

macro_optional_find_package(Prison)
macro_log_feature(PRISON_FOUND "Prison" "Prison library" "http://projects.kde.org/prison" FALSE "" "Needed to create mobile barcodes from clipboard data")
if (PRISON_FOUND)
    add_definitions(-DHAVE_PRISON)
    include_directories(${PRISON_INCLUDE_DIR})
endif (PRISON_FOUND)


kde4_add_app_icon(libklipper_common_SRCS "${KDE4_ICON_INSTALL_DIR}/oxygen/*/apps/klipper.png")
kde4_add_ui_files(libklipper_common_SRCS generalconfig.ui actionsconfig.ui editactiondialog.ui)
kde4_add_kcfg_files(libklipper_common_SRCS klippersettings.kcfgc)

set(klipper_KDEINIT_SRCS ${libklipper_common_SRCS} main.cpp tray.cpp)


kde4_add_kdeinit_executable(klipper ${klipper_KDEINIT_SRCS})

target_link_libraries(kdeinit_klipper ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES} ${ZLIB_LIBRARY})
if (X11_Xfixes_FOUND)
  target_link_libraries(kdeinit_klipper ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)
if (PRISON_FOUND)
  target_link_libraries(kdeinit_klipper ${PRISON_LIBRARIES})
endif (PRISON_FOUND)

install(TARGETS kdeinit_klipper ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS klipper         ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS klipper.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(PROGRAMS klipper.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

if(NOT WIN32)
  install( FILES klipper-kconfigxt.upd DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
endif(NOT WIN32)
