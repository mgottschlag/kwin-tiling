



########### next target ###############

set(extensiondebugger_SRCS extensiondebugger.cpp )

kde4_automoc(${extensiondebugger_SRCS})

kde4_add_executable(extensiondebugger ${extensiondebugger_SRCS})

target_link_libraries(extensiondebugger  ${KDE4_KDECORE_LIBS} kickermain )


########### next target ###############

set(appletproxy_KDEINIT_SRCS appletproxy.cpp )

kde4_automoc(${appletproxy_KDEINIT_SRCS})

kde4_add_kdeinit_executable( appletproxy ${appletproxy_KDEINIT_SRCS})

target_link_libraries(kdeinit_appletproxy  ${KDE4_KDEUI_LIBS} kickermain )

install(TARGETS kdeinit_appletproxy  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( appletproxy kdeinit_appletproxy )
install(TARGETS appletproxy DESTINATION ${BIN_INSTALL_DIR})

