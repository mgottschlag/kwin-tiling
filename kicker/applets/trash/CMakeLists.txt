include_directories( ${CMAKE_SOURCE_DIR}/libkonq ${CMAKE_SOURCE_DIR}/kicker/core   )


########### next target ###############

set(trash_panelapplet_PART_SRCS trashbutton.cpp trashapplet.cpp )

kde4_automoc(trash_panelapplet ${trash_panelapplet_PART_SRCS})

kde4_add_plugin(trash_panelapplet ${trash_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} trash_panelapplet )

target_link_libraries(trash_panelapplet  kickermain konq ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS trash_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES trashapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )




