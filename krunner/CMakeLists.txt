#KDE4_NO_ENABLE_FINAL(krunner)
macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1207)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard
${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard/processcore/)

configure_file(config-xautolock.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xautolock.h)

set(krunner_KCFG_SRCS krunnersettings.kcfgc kscreensaversettings.kcfgc klaunchsettings.kcfgc)
set(krunner_dbusXML org.kde.krunner.Interface.xml)
set(krunner_dbusAppXML org.kde.krunner.App.xml)
set(screensaver_dbusXML org.freedesktop.ScreenSaver.xml)
set(kscreensaver_dbusXML org.kde.screensaver.xml)

set(krunner_SRCS
                 krunnerdialog.cpp
                 collapsiblewidget.cpp
                 interface.cpp
                 krunnerapp.cpp
                 restartingapplication.cpp
                 main.cpp
                 saverengine.cpp
                 startupid.cpp
                 xautolock.cpp
                 xautolock_diy.c
                 xautolock_engine.c)

kde4_add_kcfg_files(krunner_SRCS ${krunner_KCFG_SRCS})

qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusXML} interface.h Interface)
qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusAppXML} krunnerapp.h KRunnerApp)
qt4_add_dbus_adaptor(krunner_SRCS ${screensaver_dbusXML} saverengine.h SaverEngine)
qt4_add_dbus_adaptor(krunner_SRCS ${kscreensaver_dbusXML} saverengine.h SaverEngine kscreensaveradaptor KScreenSaverAdaptor)

kde4_add_kdeinit_executable(krunner ${krunner_SRCS})
# needs qt3support for Q3ListView in lockdlg
target_link_libraries(kdeinit_krunner plasma kworkspace processui ${KDE4_SOLID_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} ${X11_Xrender_LIB} ${KDE4_KDEUI_LIBS} 
                      ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDE3SUPPORT_LIBRARY} )

if(X11_Xss_LIB)
  target_link_libraries(kdeinit_krunner ${X11_Xss_LIB})
endif(X11_Xss_LIB)

add_subdirectory( lock )

install(TARGETS kdeinit_krunner ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS krunner         ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES krunner.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
install(FILES kscreensaversettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES ${krunner_dbusAppXML} ${krunner_dbusXML} ${screensaver_dbusXML} ${kscreensaver_dbusXML} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
