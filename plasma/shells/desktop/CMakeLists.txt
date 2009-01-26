include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/shells/common ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)

add_subdirectory(configupdates)

set(appletbrowserdialog_SRCS
    ../common/appletbrowser.cpp
    ../common/customdragtreeview.cpp
    ../common/kcategorizeditemsview.cpp
    ../common/kcategorizeditemsviewdelegate.cpp
    ../common/kcategorizeditemsviewmodels.cpp
    ../common/openwidgetassistant.cpp
    ../common/plasmaappletitemmodel.cpp
)

set(wallpaper_SRCS
    ../common/wallpaperpreview.cpp
)

kde4_add_ui_files (
    appletbrowserdialog_SRCS
    ../common/kcategorizeditemsviewbase.ui
)

set(plasma_SRCS
    backgrounddialog.cpp
    dashboardview.cpp
    desktopcorona.cpp
    desktopview.cpp
    main.cpp
    panelcontroller.cpp
    panelview.cpp
    panelappletoverlay.cpp
    plasmaapp.cpp
    positioningruler.cpp
    toolbutton.cpp
    ${appletbrowserdialog_SRCS}
    ${wallpaper_SRCS}
)

kde4_add_ui_files(plasma_SRCS BackgroundDialog.ui)
kde4_add_kcfg_files(plasma_SRCS kcfg/plasma-shell-desktop.kcfgc)

if(WIN32)
    set(plasma_SRCS ${plasma_SRCS} panelview_win.cpp)
endif(WIN32)

set(plasmaapp_dbusXML dbus/org.kde.plasma.App.xml)
qt4_add_dbus_adaptor(plasma_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(plasma-desktop ${plasma_SRCS})

target_link_libraries(kdeinit_plasma-desktop ${KDE4_PLASMA_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS})

if(UNIX AND Q_WS_X11)
    target_link_libraries(kdeinit_plasma-desktop kworkspace ${X11_LIBRARIES})

    if(X11_Xrender_FOUND)
       target_link_libraries(kdeinit_plasma-desktop ${X11_Xrender_LIB})
    endif(X11_Xrender_FOUND)
endif(UNIX AND Q_WS_X11)

target_link_libraries(kdeinit_plasma-desktop kephal)

install(TARGETS kdeinit_plasma-desktop DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-desktop ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-desktop.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES kcfg/plasma-shell-desktop.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES plasma-themes.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})
