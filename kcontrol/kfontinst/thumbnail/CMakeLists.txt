
include_directories(
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/viewpart/
     )


set(fontthumbnail_PART_SRCS FontThumbnail.cpp )

kde4_automoc(${fontthumbnail_PART_SRCS})
kde4_add_plugin(fontthumbnail ${fontthumbnail_PART_SRCS})

target_link_libraries(fontthumbnail  ${KDE4_KIO_LIBS} kfontinstui kfontinst )

install(TARGETS fontthumbnail  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES fontthumbnail.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

