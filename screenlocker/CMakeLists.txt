# for dbus_add_activation_service
include(PkgConfigGetVar)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1223)
add_subdirectory(data)
add_subdirectory(greeter)

# for dbus_add_activation_service
include(PkgConfigGetVar)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(screensaver_dbusXML dbus/org.freedesktop.ScreenSaver.xml)
set(kscreensaver_dbusXML dbus/org.kde.screensaver.xml)
set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)

set(ksld_SRCS
   ksldapp.cpp
   autologout.cpp
   interface.cpp
   lockwindow.cpp
   main.cpp)
qt4_add_dbus_adaptor(ksld_SRCS ${screensaver_dbusXML} interface.h ScreenLocker::Interface)
qt4_add_dbus_adaptor(ksld_SRCS ${kscreensaver_dbusXML} interface.h ScreenLocker::Interface kscreensaveradaptor KScreenSaverAdaptor)
kde4_add_kcfg_files(ksld_SRCS kcfg/kscreensaversettings.kcfgc)
QT4_ADD_DBUS_INTERFACE(ksld_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(kscreenlocker ${ksld_SRCS})

target_link_libraries(kdeinit_kscreenlocker kworkspace kephal ${KDE4_KDEUI_LIBS} ${KDE4_KIDLETIME_LIBS} ${X11_LIBRARIES})

dbus_add_activation_service(dbus/org.kde.kscreenlocker.service.in)

install(TARGETS kdeinit_kscreenlocker ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kscreenlocker         ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES kscreenlocker.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
install( FILES kscreenlocker.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/kscreenlocker )
