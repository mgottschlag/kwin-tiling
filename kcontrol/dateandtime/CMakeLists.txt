


########### next target ###############

set(kcm_clock_PART_SRCS dtime.cpp tzone.cpp main.cpp )

kde4_automoc(${kcm_clock_PART_SRCS})

kde4_add_plugin(kcm_clock ${kcm_clock_PART_SRCS})


target_link_libraries(kcm_clock  ${KDE4_KIO_LIBS} )

install(TARGETS kcm_clock  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES clock.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )
