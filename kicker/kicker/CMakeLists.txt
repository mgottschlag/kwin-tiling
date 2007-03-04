
add_subdirectory( core )
add_subdirectory( buttons )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/buttons
  ${CMAKE_SOURCE_DIR}/libkonq
  ${CMAKE_CURRENT_SOURCE_DIR}/core
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
)


########### next target ###############


kde4_add_ui3_files(libkicker_ui_SRCS ui/nonKDEButtonSettings.ui)
kde4_add_ui_files(libkicker_ui_SRCS ui/appletview.ui)
qt4_add_dbus_interface(libkicker_ui_SRCS ../../krunner/org.kde.krunner.Interface.xml krunner_interface )
qt4_add_dbus_interface(libkicker_ui_SRCS ../../krunner/org.freedesktop.ScreenSaver.xml screensaver_interface )

set(kicker_KDEINIT_SRCS ${libkicker_buttons_SRCS} ${libkicker_ui_SRCS} ${libkicker_core_SRCS} )

kde4_automoc(${kicker_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kicker ${kicker_KDEINIT_SRCS})

target_link_libraries(kdeinit_kicker  ${KDE4_KIO_LIBS} kickermain ${KDE4_KUTILS_LIBS} kworkspace konq )

install(TARGETS kdeinit_kicker  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kicker kdeinit_kicker )
install(TARGETS kicker DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(kicker_reverseLayout_SRCS kicker-3.4-reverseLayout.cpp)

kde4_automoc(${kicker_reverseLayout_SRCS})

kde4_add_executable( kicker-3.4-reverseLayout ${kicker_reverseLayout_SRCS})

target_link_libraries( kicker-3.4-reverseLayout ${KDE4_KDEUI_LIBS})

install(TARGETS kicker-3.4-reverseLayout DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )

########### install files ###############

install( FILES kcmkicker.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES panel.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
install( FILES kickerrc.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install(PROGRAMS kicker-3.1-properSizeSetting.pl kicker-3.5-taskbarEnums.pl kicker-3.5-kconfigXTize.pl  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

