include_directories( ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/  )


########### next target ###############

set(kfile_font_PART_SRCS KFileFont.cpp )

kde4_automoc(${kfile_font_PART_SRCS})

kde4_add_plugin(kfile_font ${kfile_font_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_font )

target_link_libraries(kfile_font  ${KDE4_KIO_LIBS} kfontinst )

install(TARGETS kfile_font  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_font.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
