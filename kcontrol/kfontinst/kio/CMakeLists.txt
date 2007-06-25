include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/fonts)

set(kio_fonts_PART_SRCS KioFonts.cpp Server.cpp Socket.cpp SuProc.cpp ${libkxftconfig_SRCS} )
kde4_automoc(${kio_fonts_PART_SRCS})
kde4_add_plugin(kio_fonts ${kio_fonts_PART_SRCS})
# qt3support for Q3PtrList in kxftconfig.cpp
target_link_libraries(kio_fonts ${KDE4_KIO_LIBS} ${KDE4_KDESU_LIBS} ${QT_QTNETWORK_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} kfontinst )
install(TARGETS kio_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES fonts.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services )
install( FILES fonts.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )

set(kio_fonts_helper_SRCS Helper.cpp Socket.cpp ${libkxftconfig_SRCS})
kde4_add_executable(kio_fonts_helper ${kio_fonts_helper_SRCS} )
# qt3support for Q3PtrList in kxftconfig.cpp
target_link_libraries(kio_fonts_helper ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} kfontinst )
install(TARGETS kio_fonts_helper DESTINATION ${DATA_INSTALL_DIR}/kfontinst/bin )
