include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

project(krunner)

configure_file(config-xautolock.h.cmake config-xautolock.h)

set(krunner_KCFG_SRCS kscreensaversettings.kcfgc klaunchsettings.kcfgc)
set(krunner_dbusXML org.kde.krunner.Interface.xml)
set(screensaver_dbusXML org.freedesktop.ScreenSaver.xml)
set(krunner_BGIMAGES images/background/dialog.svg)

include_directories(${CMAKE_SOURCE_DIR}/workspace/plasma/lib ${CMAKE_SOURCE_DIR}/workspace/lib)

add_definitions (-DQT3_SUPPORT)

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
qt4_add_dbus_adaptor(krunner_SRCS ${screensaver_dbusXML} saverengine.h SaverEngine)

kde4_automoc(${krunner_SRCS})
kde4_add_ui_files(krunner_SRCS runners/shell/shellOptions.ui)
kde4_add_executable(krunner ${krunner_SRCS})
target_link_libraries(krunner plasma kworkspace ${KDE4_KDEUI_LIBS})
if(X11_Xss_LIB)
  target_link_libraries(krunner ${X11_Xss_LIB})
endif(X11_Xss_LIB)

add_subdirectory( lock )
add_subdirectory( runners )

install(TARGETS krunner DESTINATION ${BIN_INSTALL_DIR})
install(FILES krunner.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
install(FILES kscreensaversettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES ${krunner_BGIMAGES} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/background)
install(FILES ${krunner_dbusXML} ${screensaver_dbusXML} DESTINATION ${DBUS_INTERFACES_DIR} )
