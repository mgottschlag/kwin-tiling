include_directories( ${CMAKE_SOURCE_DIR}/workspace/kdmlib ${CMAKE_SOURCE_DIR}/workspace/kdesktop ${CMAKE_SOURCE_DIR}/workspace/kcheckpass ${CMAKE_SOURCE_DIR}/workspace/kdmlib  )

########### next target ###############

configure_file(config-kdesktop-lock.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdesktop-lock.h)

set(kdesktop_lock_SRCS ${kdmlib_SRCS}
   lockprocess.cc 
   lockdlg.cc 
   autologout.cc 
   main.cc )

set(ksmserver_xml  ${CMAKE_SOURCE_DIR}/workspace/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE( kdesktop_lock_SRCS ${ksmserver_xml} ksmserver_interface )
set(kdesktop_xml  ${CMAKE_SOURCE_DIR}/workspace/kdesktop/org.kde.kdesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE( kdesktop_lock_SRCS ${kdesktop_xml} kdesktop_interface )



kde4_add_kcfg_files(kdesktop_lock_SRCS 
	${CMAKE_SOURCE_DIR}/workspace/kdesktop/kdesktopsettings.kcfgc
	${CMAKE_SOURCE_DIR}/workspace/kdesktop/klaunchsettings.kcfgc )


kde4_automoc(${kdesktop_lock_SRCS})

kde4_add_executable(kdesktop_lock ${kdesktop_lock_SRCS} )

target_link_libraries(kdesktop_lock  ${KDE4_KIO_LIBS} )

if (HAVE_XF86MISC)
  target_link_libraries(kdesktop_lock Xxf86misc)
endif (HAVE_XF86MISC)
if(HAVE_GLXCHOOSEVISUAL)
   target_link_libraries(kdesktop_lock GL) 
endif(HAVE_GLXCHOOSEVISUAL)	

install(TARGETS kdesktop_lock DESTINATION ${BIN_INSTALL_DIR})

install_pam_service(kscreensaver)


########### install files ###############



