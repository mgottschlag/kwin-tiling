include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs 
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/desktop/shell/scripting
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal
)

add_subdirectory(configupdates)

set(activitymanager_SRCS
    activitymanager/activitymanager.cpp
    activitymanager/kidenticongenerator.cpp
)

set(plasma_SRCS
    activity.cpp
    scripting/desktopscriptengine.cpp
    scripting/panel.cpp
    checkbox.cpp
    controllerwindow.cpp
    dashboardview.cpp
    desktopcorona.cpp
    desktopview.cpp
    interactiveconsole.cpp
    main.cpp
    panelcontroller.cpp
    panelview.cpp
    panelapplethandle.cpp
    panelappletoverlay.cpp
    plasmaapp.cpp
    positioningruler.cpp
    klistconfirmationdialog.cpp
    ${wallpaper_SRCS}
    ${activitymanager_SRCS}
)

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

target_link_libraries(kdeinit_plasma-desktop ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KIDLETIME_LIBS} ${QT_QTSCRIPT_LIBRARY} ${KACTIVITIES_LIBRARY} kworkspace kactivities plasmagenericshell ${QT_QTDECLARATIVE_LIBRARY})

if(UNIX AND Q_WS_X11)
    target_link_libraries(kdeinit_plasma-desktop ${X11_LIBRARIES})

    if(X11_Xrender_FOUND)
       target_link_libraries(kdeinit_plasma-desktop ${X11_Xrender_LIB})
    endif(X11_Xrender_FOUND)
endif(UNIX AND Q_WS_X11)

target_link_libraries(kdeinit_plasma-desktop kephal)
target_link_libraries(kdeinit_plasma-desktop ${KDE4_KNEWSTUFF3_LIBS})

install(TARGETS kdeinit_plasma-desktop DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-desktop ${INSTALL_TARGETS_DEFAULT_ARGS})
if(WIN32)
install(FILES data/plasma.desktop data/plasma-desktop.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
else(WIN32)
install(FILES data/plasma.desktop data/plasma-desktop.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
endif(WIN32)
install(FILES data/plasma-shell-desktop.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES data/plasma-themes.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})
install(FILES activitymanager/activities.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})
install(FILES data/plasma-desktop.notifyrc DESTINATION ${DATA_INSTALL_DIR}/plasma-desktop)
install(FILES data/layouts/00-defaultLayout.js DESTINATION ${DATA_INSTALL_DIR}/plasma-desktop/init)
install(FILES data/plasma-desktop-js.xml DESTINATION ${DATA_INSTALL_DIR}/katepart/syntax/)
install(DIRECTORY activitymanager/package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.desktop.activitymanager)

MACRO(INSTALL_PLASMA_JAVASCRIPT_TEMPLATE dir addonPrefix)
   configure_file(${dir}/metadata.desktop ${CMAKE_CURRENT_BINARY_DIR}/plasma-layout-${addonPrefix}.desktop COPYONLY)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-layout-${addonPrefix}.desktop DESTINATION ${SERVICES_INSTALL_DIR})
   install(FILES ${dir}/metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/${addonPrefix}/)
   install(FILES ${dir}/contents/layout.js DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/${addonPrefix}/contents/)
ENDMACRO(INSTALL_PLASMA_JAVASCRIPT_TEMPLATE)

INSTALL_PLASMA_JAVASCRIPT_TEMPLATE(data/layouts/org.kde.plasma-desktop.defaultPanel org.kde.plasma-desktop.defaultPanel)
INSTALL_PLASMA_JAVASCRIPT_TEMPLATE(data/layouts/org.kde.plasma-desktop.findWidgets org.kde.plasma-desktop.findWidgets)
INSTALL_PLASMA_JAVASCRIPT_TEMPLATE(data/layouts/org.kde.plasma-desktop.photoActivity org.kde.plasma-desktop.photoActivity)
INSTALL_PLASMA_JAVASCRIPT_TEMPLATE(data/layouts/org.kde.plasma-desktop.desktopIcons org.kde.plasma-desktop.desktopIcons)
INSTALL_PLASMA_JAVASCRIPT_TEMPLATE(data/layouts/org.kde.plasma-desktop.SaL org.kde.plasma-desktop.SaL)

