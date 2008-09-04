#KDE4_NO_ENABLE_FINAL(krunner)
macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
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
   configdialog.cpp
   interface.cpp
   krunnerapp.cpp
   main.cpp
   resultitem.cpp
   resultscene.cpp
   stackblur_shadows.cpp)
if(NOT WIN32)
set(krunner_SRCS
    ${krunner_SRCS}
    saverengine.cpp
    startupid.cpp
    xautolock.cpp
    xautolock_diy.c
    xautolock_engine.c)
endif(NOT WIN32)
    

kde4_add_kcfg_files(krunner_SRCS ${krunner_KCFG_SRCS})

qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusXML} interface.h Interface)
qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusAppXML} krunnerapp.h KRunnerApp)
if(NOT WIN32)
qt4_add_dbus_adaptor(krunner_SRCS ${screensaver_dbusXML} saverengine.h SaverEngine)
qt4_add_dbus_adaptor(krunner_SRCS ${kscreensaver_dbusXML} saverengine.h SaverEngine kscreensaveradaptor KScreenSaverAdaptor)
endif(NOT WIN32)
dbus_add_activation_service(org.kde.krunner.service.in)

kde4_add_kdeinit_executable(krunner ${krunner_SRCS})
if(NOT WIN32)
target_link_libraries(kdeinit_krunner plasma kworkspace processui ${KDE4_SOLID_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} ${X11_Xrender_LIB} ${KDE4_KDEUI_LIBS}  ${KDE4_KUTILS_LIBS} ${X11_LIBRARIES})
else(NOT WIN32)
target_link_libraries(kdeinit_krunner plasma  ${KDE4_SOLID_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} ${KDE4_KDEUI_LIBS}  ${KDE4_KUTILS_LIBS})
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
install(FILES ${krunner_dbusAppXML} ${krunner_dbusXML} ${screensaver_dbusXML} ${kscreensaver_dbusXML} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
