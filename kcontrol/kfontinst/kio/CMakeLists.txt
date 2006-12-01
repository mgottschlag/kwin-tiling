include_directories(
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/fonts/
     )


set(kio_fonts_PART_SRCS KioFonts.cpp ${libkxftconfig_SRCS} )

kde4_automoc(${kio_fonts_PART_SRCS})
kde4_add_plugin(kio_fonts ${kio_fonts_PART_SRCS})

target_link_libraries(kio_fonts  ${KDE4_KIO_LIBS} ${KDE4_KDESU_LIBS} kfontinst )

install(TARGETS kio_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES fonts.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services )
install( FILES folder.desktop system-folder.desktop package.desktop  DESTINATION  ${MIME_INSTALL_DIR}/fonts )
install( FILES fonts.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
