include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/shells/common)

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
    wallpaperpreview.cpp
    ${appletbrowserdialog_SRCS}
)

kde4_add_ui_files(plasma_SRCS BackgroundDialog.ui)

if(WIN32)
    set(plasma_SRCS ${plasma_SRCS} panelview_win.cpp)
endif(WIN32)

set(plasmaapp_dbusXML org.kde.plasma.App.xml)
qt4_add_dbus_adaptor(plasma_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(plasma_qgv ${plasma_SRCS})

if(WIN32)
target_link_libraries(kdeinit_plasma_qgv ${PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS})
else(WIN32)
target_link_libraries(kdeinit_plasma_qgv ${PLASMA_LIBS} kworkspace ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${X11_LIBRARIES})
endif(WIN32)
if(X11_Xrender_FOUND)
  target_link_libraries(kdeinit_plasma_qgv ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)
set_target_properties(kdeinit_plasma_qgv PROPERTIES OUTPUT_NAME kdeinit4_plasma)
set_target_properties(plasma_qgv PROPERTIES OUTPUT_NAME plasma)

install(TARGETS kdeinit_plasma_qgv DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma_qgv ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

install(FILES plasma-themes.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})
