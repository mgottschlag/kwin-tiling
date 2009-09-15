include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/shells/common ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/desktop/shell/scripting/ ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)

add_subdirectory(configupdates)

set(appletbrowserdialog_SRCS
    ../../shells/common/widgetsExplorer/kcategorizeditemsviewmodels.cpp
    ../../shells/common/widgetsExplorer/plasmaappletitemmodel.cpp
    ../../shells/common/widgetsExplorer/appleticon.cpp
    ../../shells/common/widgetsExplorer/appletsfiltering.cpp
    ../../shells/common/widgetsExplorer/applettooltip.cpp
    ../../shells/common/widgetsExplorer/appletslist.cpp
    ../../shells/common/widgetsExplorer/managewidgets.cpp
    ../../shells/common/widgetsExplorer/widgetexplorer.cpp
    ../../shells/common/widgetsExplorer/openwidgetassistant.cpp
)

set(plasma_SRCS
    scripting/containment.cpp
    scripting/rect.cpp
    scripting/scriptengine.cpp
    scripting/widget.cpp
    checkbox.cpp
    controllerwindow.cpp
    dashboardview.cpp
    desktopcorona.cpp
    desktopview.cpp
    interactiveconsole.cpp
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

kde4_add_ui_files(plasma_SRCS globaloptions.ui)
kde4_add_kcfg_files(plasma_SRCS data/plasma-shell-desktop.kcfgc)

if(WIN32)
    set(plasma_SRCS ${plasma_SRCS} panelview_win.cpp)
endif(WIN32)

set(plasmaapp_dbusXML dbus/org.kde.plasma.App.xml)
qt4_add_dbus_adaptor(plasma_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_app_icon(plasma_SRCS "${KDE4_INSTALL_DIR}/share/icons/oxygen/*/apps/plasma.png")

kde4_add_kdeinit_executable(plasma-desktop ${plasma_SRCS})

target_link_libraries(kdeinit_plasma-desktop ${KDE4_PLASMA_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTSCRIPT_LIBRARY} kworkspace plasmagenericshell)

if(UNIX AND Q_WS_X11)
    target_link_libraries(kdeinit_plasma-desktop ${X11_LIBRARIES})

    if(X11_Xrender_FOUND)
       target_link_libraries(kdeinit_plasma-desktop ${X11_Xrender_LIB})
    endif(X11_Xrender_FOUND)
endif(UNIX AND Q_WS_X11)

target_link_libraries(kdeinit_plasma-desktop kephal)

install(TARGETS kdeinit_plasma-desktop DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-desktop ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES data/plasma.desktop data/plasma-desktop.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES data/plasma-shell-desktop.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES data/plasma-themes.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})
install(FILES data/plasma-desktop.notifyrc DESTINATION ${DATA_INSTALL_DIR}/plasma-desktop)
install(FILES data/plasma-desktop-js.xml DESTINATION ${DATA_INSTALL_DIR}/katepart/syntax/)
