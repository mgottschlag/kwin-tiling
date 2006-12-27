
project(kcminit)



########### next target ###############

set( kcminit_SRCS)

if(Q_WS_X11)
	set(ksplash_xml  ${CMAKE_SOURCE_DIR}/workspace/ksplashml/org.kde.KSplash.xml)
	QT4_ADD_DBUS_INTERFACE( kcminit_SRCS  ${ksplash_xml} ksplash_interface )
endif(Q_WS_X11)


set(kcminit_KDEINIT_SRCS main.cpp ${kcminit_SRCS})

kde4_automoc(${kcminit_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcminit ${kcminit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit  ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS kdeinit_kcminit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kcminit kdeinit_kcminit )
install(TARGETS kcminit DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

# TODO might be simpler to make <whatever>_startup to be a symlink to <whatever>

set(kcminit_startup_KDEINIT_SRCS main.cpp ${kcminit_SRCS})

kde4_automoc(${kcminit_startup_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcminit_startup ${kcminit_startup_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit_startup  ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS kdeinit_kcminit_startup  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kcminit_startup kdeinit_kcminit_startup )
install(TARGETS kcminit_startup DESTINATION ${BIN_INSTALL_DIR})

