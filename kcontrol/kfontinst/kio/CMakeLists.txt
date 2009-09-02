
include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/fonts
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/kcmfontinst
                    ${CMAKE_CURRENT_BINARY_DIR})

set(kio_fonts_PART_SRCS KioFonts.cpp Server.cpp Socket.cpp SuProc.cpp ${libkxftconfig_SRCS})
set(kio_fonts_helper_SRCS Helper.cpp Socket.cpp ${libkxftconfig_SRCS})
kde4_add_executable(kio_fonts_helper ${kio_fonts_helper_SRCS} )
target_link_libraries(kio_fonts_helper ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY}
                                       ${QT_QTGUI_LIBRARY} ${FONTCONFIG_LIBRARIES} kfontinst )
install(TARGETS kio_fonts_helper DESTINATION ${LIBEXEC_INSTALL_DIR} )

kde4_add_plugin(kio_fonts ${kio_fonts_PART_SRCS} ${KFI_FONTINST_AUTH_SRC} )
target_link_libraries(kio_fonts ${KDE4_KIO_LIBS} ${KDE4_KDESU_LIBS} ${QT_QTNETWORK_LIBRARY} ${FONTCONFIG_LIBRARIES} ${FREETYPE_LIBRARIES} kfontinst )
install(TARGETS kio_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES fonts.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services )
install( FILES fonts.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
kde4_install_icons( ${ICON_INSTALL_DIR}  )

