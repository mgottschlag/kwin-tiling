include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs)

set(plasma_SRCS
    desktopcorona.cpp
    desktopview.cpp
    main.cpp
    plasmaapp.cpp
    rootwidget.cpp
    panelview.cpp
    dashboardview.cpp
)

set(plasmaapp_dbusXML org.kde.plasma.App.xml)
qt4_add_dbus_adaptor(plasma_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(plasma_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(plasma-qgv ${plasma_SRCS})

target_link_libraries(kdeinit_plasma-qgv plasma kworkspace  ${KDE4_KIO_LIBS})
set_target_properties(kdeinit_plasma-qgv PROPERTIES OUTPUT_NAME kdeinit4_plasma)
set_target_properties(plasma-qgv PROPERTIES OUTPUT_NAME plasma)

install(TARGETS kdeinit_plasma-qgv DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-qgv DESTINATION ${BIN_INSTALL_DIR})
install( FILES plasma.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
