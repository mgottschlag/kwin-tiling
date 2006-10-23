
project(ksmserver)
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kdmlib  )

configure_file(config-ksmserver.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksmserver.h)

########### next target ###############

set(ksmserver_KDEINIT_SRCS ${kdmlib_SRCS} 
   main.cpp 
   server.cpp 
   shutdowndlg.cpp 
   legacy.cpp 
   startup.cpp 
   shutdown.cpp 
   client.cpp)

qt4_add_dbus_adaptor( ksmserver_KDEINIT_SRCS org.kde.KSMServerInterface.xml server.h KSMServer )

kde4_automoc(${ksmserver_KDEINIT_SRCS})


kde4_add_kdeinit_executable( ksmserver ${ksmserver_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksmserver ${KDE4_KDEUI_LIBS} ${KDE4_KDEFX_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${X11_LIBRARIES})

install(TARGETS kdeinit_ksmserver  DESTINATION ${LIB_INSTALL_DIR})

target_link_libraries( ksmserver kdeinit_ksmserver )
install(TARGETS ksmserver DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

if(KDE4_BUILD_TESTS)
set(testsh_SRCS ${kdmlib_SRCS} test.cpp shutdowndlg.cpp )

kde4_automoc(${testsh_SRCS})


kde4_add_executable(testsh ${testsh_SRCS})

target_link_libraries(testsh ${KDE4_KDEUI_LIBS})

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES shutdownkonq.png  DESTINATION  ${DATA_INSTALL_DIR}/ksmserver/pics )
install( FILES ksmserver.upd DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS move_session_config.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

