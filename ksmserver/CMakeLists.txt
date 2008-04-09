add_subdirectory( kcm )
add_subdirectory( windowmanagers )
add_subdirectory( tests )

include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/solid/control/
   ${BLITZ_INCLUDES}
)
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
add_custom_command(OUTPUT ${kcminit_xml}
       COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${kcminit_adaptor} > ${kcminit_xml}
       DEPENDS ${kcminit_adaptor})
qt4_add_dbus_interface( ksmserver_KDEINIT_SRCS ${kcminit_xml} kcminit_interface )

# KLauchner.xml is installed by kdelibs, so it is in KDE4_DBUS_INTERFACES_DIR
set(klauncher_xml  ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KLauncher.xml)
qt4_add_dbus_interface( ksmserver_KDEINIT_SRCS ${klauncher_xml} klauncher_interface )

qt4_add_dbus_adaptor( ksmserver_KDEINIT_SRCS org.kde.KSMServerInterface.xml server.h KSMServer )

kde4_add_kdeinit_executable( ksmserver ${ksmserver_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksmserver plasma solidcontrol kworkspace ${KDE4_KDEUI_LIBS} ${BLITZ_LIBRARIES} ${X11_LIBRARIES})

install(TARGETS kdeinit_ksmserver ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( ksmserver kdeinit_ksmserver )
install(TARGETS ksmserver DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(kcheckrunning_SRCS
   kcheckrunning.cpp)

kde4_add_executable( kcheckrunning ${kcheckrunning_SRCS})

target_link_libraries(kcheckrunning ${X11_LIBRARIES})

install(TARGETS kcheckrunning DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install( FILES ksmserver.upd DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS move_session_config.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES org.kde.KSMServerInterface.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
