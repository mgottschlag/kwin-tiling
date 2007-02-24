include_directories( ${CMAKE_SOURCE_DIR}/workspace/krunner ${CMAKE_SOURCE_DIR}/workspace/kcheckpass ${CMAKE_SOURCE_DIR}/workspace/kdmlib )

########### next target ###############

configure_file(config-krunner-lock.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-krunner-lock.h)

set(krunner_lock_SRCS
   lockprocess.cc
   lockdlg.cc
   autologout.cc
   main.cc )

set(ksmserver_xml  ${CMAKE_SOURCE_DIR}/workspace/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(krunner_lock_SRCS ${ksmserver_xml} ksmserver_interface)
set(krunner_xml  ${CMAKE_SOURCE_DIR}/workspace/krunner/org.kde.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(krunner_lock_SRCS ${krunner_xml} krunner_interface)

set(kxkb_adaptor ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kxkb/kxkb_adaptor.h)
set(kxkb_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KXKB.xml)
ADD_CUSTOM_COMMAND(OUTPUT ${kxkb_xml}
       COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${kxkb_adaptor} > ${kxkb_xml}
       DEPENDS ${kxkb_adaptor})
QT4_ADD_DBUS_INTERFACE( krunner_lock_SRCS ${kxkb_xml} kxkb_interface )



kde4_add_kcfg_files(krunner_lock_SRCS ${CMAKE_SOURCE_DIR}/workspace/krunner/kscreensaversettings.kcfgc)


kde4_automoc(${krunner_lock_SRCS})

kde4_add_executable(krunner_lock ${krunner_lock_SRCS} )

target_link_libraries(krunner_lock  ${KDE4_KIO_LIBS} kworkspace )

if (HAVE_XF86MISC)
  target_link_libraries(krunner_lock Xxf86misc)
endif (HAVE_XF86MISC)
if(HAVE_GLXCHOOSEVISUAL)
   target_link_libraries(krunner_lock GL)
endif(HAVE_GLXCHOOSEVISUAL)

install(TARGETS krunner_lock DESTINATION ${BIN_INSTALL_DIR})

install_pam_service(kscreensaver)


########### install files ###############



