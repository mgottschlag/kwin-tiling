
add_subdirectory( pics ) 
include_directories( ${X11_dpms_INCLUDE_PATH}   )


########### next target ###############

set(kcm_energy_PART_SRCS energy.cpp )

kde4_automoc(${kcm_energy_PART_SRCS})

kde4_add_plugin(kcm_energy ${kcm_energy_PART_SRCS})


target_link_libraries(kcm_energy  ${KDE4_KIO_LIBS} )

install(TARGETS kcm_energy  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES energy.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )
