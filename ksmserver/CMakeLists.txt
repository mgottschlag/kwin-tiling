
project(ksmserver)
include_directories( ${CMAKE_SOURCE_DIR}/workspace/lib ${CMAKE_SOURCE_DIR}/plasma/lib )

configure_file(config-ksmserver.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksmserver.h)

########### next target ###############

set(ksmserver_KDEINIT_SRCS
   main.cpp 
   server.cpp 
   shutdowndlg.cpp 
   legacy.cpp 
   startup.cpp 
   shutdown.cpp 
   client.cpp)

set(kcminit_adaptor ${CMAKE_CURRENT_SOURCE_DIR}/../kcminit/main.h)
set(kcminit_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCMinit.xml)
ADD_CUSTOM_COMMAND(OUTPUT ${kcminit_xml}
       COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${kcminit_adaptor} > ${kcminit_xml}
       DEPENDS ${kcminit_adaptor})
QT4_ADD_DBUS_INTERFACE( ksmserver_KDEINIT_SRCS ${kcminit_xml} kcminit_interface )


#set(kdesktop_xml  ${CMAKE_SOURCE_DIR}/workspace/kdesktop/org.kde.kdesktop.Desktop.xml)
#QT4_ADD_DBUS_INTERFACE( ksmserver_KDEINIT_SRCS ${kdesktop_xml} kdesktop_interface )

set(ksplash_xml  ${CMAKE_SOURCE_DIR}/workspace/ksplashml/org.kde.KSplash.xml)
QT4_ADD_DBUS_INTERFACE( ksmserver_KDEINIT_SRCS ${ksplash_xml} ksplash_interface )

set(klauncher_xml  ${DBUS_INTERFACES_DIR}/org.kde.KLauncher.xml)
QT4_ADD_DBUS_INTERFACE( ksmserver_KDEINIT_SRCS ${klauncher_xml} klauncher_interface )


qt4_add_dbus_adaptor( ksmserver_KDEINIT_SRCS org.kde.KSMServerInterface.xml server.h KSMServer )

kde4_automoc(${ksmserver_KDEINIT_SRCS})


kde4_add_kdeinit_executable( ksmserver ${ksmserver_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksmserver plasma kworkspace ${KDE4_KDEUI_LIBS} ${KDE4_KDEFX_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${X11_LIBRARIES})

install(TARGETS kdeinit_ksmserver  DESTINATION ${LIB_INSTALL_DIR})

target_link_libraries( ksmserver kdeinit_ksmserver )
install(TARGETS ksmserver DESTINATION ${BIN_INSTALL_DIR})

set(ksmserver_BGIMAGES images/background/shutdowndlg.svg)
install(FILES ${ksmserver_BGIMAGES} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/background)

########### next target ###############

if(KDE4_BUILD_TESTS)
set(testsh_SRCS test.cpp shutdowndlg.cpp )

kde4_automoc(${testsh_SRCS})


kde4_add_executable(testsh ${testsh_SRCS})

target_link_libraries(testsh plasma kworkspace ${KDE4_KDEUI_LIBS})

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES shutdownkonq.png  DESTINATION  ${DATA_INSTALL_DIR}/ksmserver/pics )
install( FILES ksmserver.upd DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS move_session_config.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES org.kde.KSMServerInterface.xml DESTINATION ${DBUS_INTERFACES_DIR})
