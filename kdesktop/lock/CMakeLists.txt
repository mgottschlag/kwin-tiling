include_directories( ${CMAKE_SOURCE_DIR}/workspace/kdmlib ${CMAKE_SOURCE_DIR}/workspace/kdesktop ${CMAKE_SOURCE_DIR}/workspace/kcheckpass ${CMAKE_SOURCE_DIR}/workspace/kdmlib  )

########### next target ###############

set(kdesktop_lock_SRCS ${kdmlib_SRCS}
   lockprocess.cc 
   lockdlg.cc 
   autologout.cc 
   main.cc )

kde4_add_kcfg_files(kdesktop_lock_SRCS 
	${CMAKE_SOURCE_DIR}/workspace/kdesktop/kdesktopsettings.kcfgc
	${CMAKE_SOURCE_DIR}/workspace/kdesktop/klaunchsettings.kcfgc )


kde4_automoc(${kdesktop_lock_SRCS})

kde4_add_executable(kdesktop_lock ${kdesktop_lock_SRCS} )

target_link_libraries(kdesktop_lock  ${KDE4_KIO_LIBS} )

if (HAVE_XF86MISC)
  target_link_libraries(kdesktop_lock Xxf86misc)
endif (HAVE_XF86MISC)

install(TARGETS kdesktop_lock DESTINATION ${BIN_INSTALL_DIR})

install_pam_service(kscreensaver)


########### install files ###############



