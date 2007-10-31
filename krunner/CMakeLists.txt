#KDE4_NO_ENABLE_FINAL(krunner)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/libksysguard)

set(krunner_KCFG_SRCS kscreensaversettings.kcfgc klaunchsettings.kcfgc)
set(krunner_dbusXML org.kde.krunner.Interface.xml)
set(krunner_dbusAppXML org.kde.krunner.App.xml)
set(screensaver_dbusXML org.freedesktop.ScreenSaver.xml)

set(krunner_SRCS
                 krunnerdialog.cpp
                 collapsiblewidget.cpp
                 interface.cpp
                 krunnerapp.cpp
                 restartingapplication.cpp
                 main.cpp
                 runners/services/servicerunner.cpp
                 runners/sessions/sessionrunner.cpp
                 runners/shell/shellrunner.cpp
                 saverengine.cpp
                 startupid.cpp
                 xautolock.cpp
                 xautolock_diy.c
                 xautolock_engine.c)

kde4_add_kcfg_files(krunner_SRCS ${krunner_KCFG_SRCS})

qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusXML} interface.h Interface)
qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusAppXML} krunnerapp.h KRunnerApp)
qt4_add_dbus_adaptor(krunner_SRCS ${screensaver_dbusXML} saverengine.h SaverEngine)

kde4_add_ui_files(krunner_SRCS runners/shell/shellOptions.ui)
kde4_add_executable(krunner ${krunner_SRCS})
# needs qt3support for Q3ListView in lockdlg
target_link_libraries(krunner plasma kworkspace processui ${X11_Xrender_LIB} ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDE3SUPPORT_LIBRARY} )
if(X11_Xss_LIB)
  target_link_libraries(krunner ${X11_Xss_LIB})
endif(X11_Xss_LIB)

add_subdirectory( lock )

install(TARGETS krunner DESTINATION ${BIN_INSTALL_DIR})
install(FILES krunner.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
install(FILES kscreensaversettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES ${krunner_dbusAppXML} ${krunner_dbusXML} ${screensaver_dbusXML} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
