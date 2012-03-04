#KDE4_NO_ENABLE_FINAL(krunner)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1207)

# for dbus_add_activation_service
include(PkgConfigGetVar)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard/processcore
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/screensaver
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)

macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
macro_log_feature(HAVE_XSCREENSAVER "libxss" "XScreenSaver Library" "http://www.x.org/" FALSE "" "Needed to enable screensaver status check")
configure_file(config-xautolock.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xautolock.h)

set(krunner_KCFG_SRCS kcfg/krunnersettings.kcfgc kcfg/kscreensaversettings.kcfgc kcfg/klaunchsettings.kcfgc)
set(krunner_dbusAppXML dbus/org.kde.krunner.App.xml)
set(screensaver_dbusXML dbus/org.freedesktop.ScreenSaver.xml)
set(kscreensaver_dbusXML dbus/org.kde.screensaver.xml)


set(default_interface_SRCS
    interfaces/default/interface.cpp
    interfaces/default/resultitem.cpp
    interfaces/default/resultscene.cpp
    interfaces/default/resultview.cpp
    interfaces/default/krunnerhistorycombobox.cpp
    interfaces/default/selectionbar.cpp
)

set(quicksand_interface_SRCS
   interfaces/quicksand/qs_completionbox.cpp
   interfaces/quicksand/qs_dialog.cpp
   interfaces/quicksand/qs_matchitem.cpp
   interfaces/quicksand/qs_matchview.cpp
   interfaces/quicksand/qs_queryactionitem.cpp
   interfaces/quicksand/qs_querymatchitem.cpp
   interfaces/quicksand/qs_statusbar.cpp)

set(krunner_SRCS
   krunnerdialog.cpp
   configdialog.cpp
   krunnerapp.cpp
   main.cpp
   ${default_interface_SRCS}
   ${quicksand_interface_SRCS})

kde4_add_ui_files(krunner_SRCS interfaceOptions.ui)
kde4_add_kcfg_files(krunner_SRCS ${krunner_KCFG_SRCS})

qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusAppXML} krunnerapp.h KRunnerApp)
if(WIN32)
	set(BIN_PREFIX bin)
else(WIN32)
	set(BIN_PREFIX ${CMAKE_INSTALL_PREFIX}/bin)
endif(WIN32)

dbus_add_activation_service(dbus/org.kde.krunner.service.in)

if(NOT WIN32)
set(krunner_SRCS
    ${krunner_SRCS}
    ksystemactivitydialog.cpp
    startupid.cpp
    screensaver/saverengine.cpp
    screensaver/xautolock.cpp
    screensaver/xautolock_diy.c
    screensaver/xautolock_engine.c)

qt4_add_dbus_adaptor(krunner_SRCS ${screensaver_dbusXML} saverengine.h SaverEngine)
qt4_add_dbus_adaptor(krunner_SRCS ${kscreensaver_dbusXML} saverengine.h SaverEngine kscreensaveradaptor KScreenSaverAdaptor)
endif(NOT WIN32)

kde4_add_kdeinit_executable(krunner ${krunner_SRCS})

target_link_libraries(kdeinit_krunner kworkspace  plasmagenericshell kephal ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} ${KDE4_KDEUI_LIBS}  ${KDE4_KCMUTILS_LIBS} )

if(NOT WIN32)
  target_link_libraries(kdeinit_krunner processui  ${X11_Xrender_LIB} ${X11_LIBRARIES})
endif(NOT WIN32)

if(X11_Xss_LIB)
  target_link_libraries(kdeinit_krunner ${X11_Xss_LIB})
endif(X11_Xss_LIB)

if(X11_Xcursor_FOUND)
  target_link_libraries(kdeinit_krunner ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

if(NOT WIN32)
add_subdirectory( lock )
endif(NOT WIN32)

install(TARGETS kdeinit_krunner ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS krunner         ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES krunner.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
install(FILES kcfg/kscreensaversettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES kcfg/klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES ${krunner_dbusAppXML} ${screensaver_dbusXML} ${kscreensaver_dbusXML} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
