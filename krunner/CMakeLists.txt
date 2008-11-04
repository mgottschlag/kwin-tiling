#KDE4_NO_ENABLE_FINAL(krunner)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1207)

include(UsePkgConfig)

MACRO(PKGCONFIG_GETVAR _package _var _output_variable)
   SET(${_output_variable})

   # if pkg-config has been found
   IF(PKGCONFIG_EXECUTABLE)
      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
      # and if the package of interest also exists for pkg-config, then get the information
      IF(NOT _return_VALUE)
         EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable ${_var} OUTPUT_VARIABLE ${_output_variable} )
      ENDIF(NOT _return_VALUE)
   ENDIF(PKGCONFIG_EXECUTABLE)
ENDMACRO(PKGCONFIG_GETVAR _package _var _output_variable)

macro(dbus_add_activation_service _sources)
   #PKGCONFIG_GETVAR(dbus-1 session_bus_services_dir _install_dir)
   foreach (_i ${_sources})
      get_filename_component(_service_file ${_i} ABSOLUTE)
      string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
      set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
      configure_file(${_service_file} ${_target})
      install(FILES ${_target} DESTINATION ${DBUS_SERVICES_INSTALL_DIR} )#${_install_dir})
   endforeach (_i ${ARGN})
endmacro(dbus_add_activation_service _sources)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard/processcore/
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/screensaver)

configure_file(config-xautolock.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xautolock.h)

set(krunner_KCFG_SRCS krunnersettings.kcfgc kscreensaversettings.kcfgc klaunchsettings.kcfgc)
set(krunner_dbusAppXML dbus/org.kde.krunner.App.xml)
set(screensaver_dbusXML dbus/org.freedesktop.ScreenSaver.xml)
set(kscreensaver_dbusXML dbus/org.kde.screensaver.xml)

set(default_interface_SRCS
   interfaces/default/resultitem.cpp
   interfaces/default/resultscene.cpp
   interfaces/default/interface.cpp)

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

kde4_add_kcfg_files(krunner_SRCS ${krunner_KCFG_SRCS})

qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusAppXML} krunnerapp.h KRunnerApp)
dbus_add_activation_service(dbus/org.kde.krunner.service.in)

if(NOT WIN32)
set(krunner_SRCS
    ${krunner_SRCS}
    startupid.cpp
    screensaver/saverengine.cpp
    screensaver/xautolock.cpp
    screensaver/xautolock_diy.c
    screensaver/xautolock_engine.c)

qt4_add_dbus_adaptor(krunner_SRCS ${screensaver_dbusXML} saverengine.h SaverEngine)
qt4_add_dbus_adaptor(krunner_SRCS ${kscreensaver_dbusXML} saverengine.h SaverEngine kscreensaveradaptor KScreenSaverAdaptor)
endif(NOT WIN32)

kde4_add_kdeinit_executable(krunner ${krunner_SRCS})

if(NOT WIN32)
target_link_libraries(kdeinit_krunner kworkspace processui ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} ${X11_Xrender_LIB} ${KDE4_KDEUI_LIBS}  ${KDE4_KUTILS_LIBS} ${X11_LIBRARIES})
else(NOT WIN32)
target_link_libraries(kdeinit_krunner ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} ${KDE4_KDEUI_LIBS}  ${KDE4_KUTILS_LIBS})
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
install(FILES kscreensaversettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES ${krunner_dbusAppXML} ${screensaver_dbusXML} ${kscreensaver_dbusXML} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
