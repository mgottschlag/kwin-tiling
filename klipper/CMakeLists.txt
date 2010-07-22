kde4_no_enable_final(klipper)

find_package(Dmtx)

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
    actionstreewidget.cpp
    editactiondialog.cpp
    clipcommandprocess.cpp
)

if(DMTX_FOUND)
    MESSAGE(STATUS "Found Datamatrix library, will use it to give possibility to show mobile barcodes of clipboard data")
    set(libklipper_common_SRCS ${libklipper_common_SRCS} mobilebarcode.cpp)
    add_definitions(-DHAVE_DMTX)
else(DMTX_FOUND)
    MESSAGE(STATUS "Datamatrix library not found. Consider installing it to create mobile barcodes from clipboard data. See http://wwwlibdmtx.org")
endif(DMTX_FOUND)

macro_log_feature(DMTX_FOUND "Dmtx" "Datamatrix library" "http://www.libdmtx.org" FALSE "" "Needed to provide create mobile barcodes from clipboard data")
include_directories(${QNTRACK_INCLUDE_DIR})


kde4_add_app_icon(libklipper_common_SRCS "${KDE4_ICON_INSTALL_DIR}/oxygen/*/apps/klipper.png")
kde4_add_ui_files(libklipper_common_SRCS generalconfig.ui actionsconfig.ui editactiondialog.ui)
kde4_add_kcfg_files(libklipper_common_SRCS klippersettings.kcfgc)

set(klipper_KDEINIT_SRCS ${libklipper_common_SRCS} main.cpp tray.cpp)


kde4_add_kdeinit_executable(klipper ${klipper_KDEINIT_SRCS})

target_link_libraries(kdeinit_klipper ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES} ${ZLIB_LIBRARY})
if (X11_Xfixes_FOUND)
  target_link_libraries(kdeinit_klipper ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)
if (DMTX_FOUND)
  target_link_libraries(kdeinit_klipper ${DMTX_LIBRARIES})
endif (DMTX_FOUND)

install(TARGETS kdeinit_klipper ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS klipper         ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES klipper.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES klipper.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

if(NOT WIN32)
  install( FILES klipper-kconfigxt.upd DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
endif(NOT WIN32)
