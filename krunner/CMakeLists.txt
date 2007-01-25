include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

project(krunner)

configure_file(config-xautolock.h.cmake config-xautolock.h)

set(krunner_KCFG_SRCS kscreensaversettings.kcfgc klaunchsettings.kcfgc)
set(krunner_dbusXML org.kde.krunner.Interface.xml)
set(screensaver_dbusXML org.kde.ScreenSaver.xml)
set(krunner_BGIMAGES images/background/dialog.svg)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/plasma/lib)

set(krunner_SRCS interface.cpp
                 runner.cpp
                 apprunner.cpp
                 restartingapplication.cpp
                 searchrunner.cpp
                 shellrunner.cpp
                 main.cpp
                 xautolock.cpp
                 xautolock_diy.c
                 xautolock_engine.c
                 saverengine.cpp
                 startupid.cpp)

kde4_add_kcfg_files(krunner_SRCS ${krunner_KCFG_SRCS})

qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusXML} interface.h Interface)
qt4_add_dbus_adaptor(krunner_SRCS ${screensaver_dbusXML} saverengine.h SaverEngine)

kde4_automoc(${krunner_SRCS})
kde4_add_executable(krunner ${krunner_SRCS})
target_link_libraries(krunner plasma ${KDE4_KDEUI_LIBS} ${X11_Xss_LIB})

add_subdirectory( lock )

install(TARGETS krunner DESTINATION ${BIN_INSTALL_DIR})
install(FILES krunner.desktop  DESTINATION  ${SERVICES_INSTALL_DIR})
install(FILES kscreensaversettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES ${krunner_BGIMAGES} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/background)
install(FILES ${krunner_dbusXML} ${screensaver_dbusXML} DESTINATION ${DBUS_INTERFACES_DIR} )
