########### next target ###############

set(kded_randrmonitor_PART_SRCS randrmonitor.cpp )

kde4_add_plugin(kded_randrmonitor ${kded_randrmonitor_PART_SRCS})

target_link_libraries(kded_randrmonitor ${KDE4_KDEUI_LIBS} ${X11_Xrandr_LIB} ${X11_LIBRARIES})

install(TARGETS kded_randrmonitor  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES randrmonitor.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
