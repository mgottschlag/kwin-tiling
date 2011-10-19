include_directories(${CMAKE_SOURCE_DIR} 
                    ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/fonts)
add_definitions(${QT_DEFINITIONS})

set(fontinst_bin_SRCS FcConfig.cpp FontInst.cpp Folder.cpp Main.cpp Utils.cpp ${libkfontinstdbusiface_SRCS} )
set(fontinst_helper_SRCS FcConfig.cpp Helper.cpp Folder.cpp Utils.cpp ${libkfontinstdbusiface_SRCS} )

# qt4_generate_dbus_interface(FontInst.h org.kde.fontinst.xml)
qt4_add_dbus_adaptor(fontinst_bin_SRCS org.kde.fontinst.xml FontInst.h KFI::FontInst)
# qt4_add_dbus_interface(fontinst_bin_SRCS org.kde.fontinst.xml FontinstIface)

kde4_add_executable(fontinst_bin ${fontinst_bin_SRCS})
kde4_add_executable(fontinst_helper ${fontinst_helper_SRCS})

set_target_properties(fontinst_bin PROPERTIES OUTPUT_NAME fontinst)
target_link_libraries(fontinst_bin ${KDE4_KDECORE_LIBS}
                      ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${FONTCONFIG_LIBRARIES} kfontinst)

set_target_properties(fontinst_helper PROPERTIES OUTPUT_NAME fontinst_helper)
target_link_libraries(fontinst_helper ${KDE4_KDECORE_LIBS}
                      ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${FONTCONFIG_LIBRARIES} kfontinst)

install(TARGETS fontinst_bin DESTINATION ${LIBEXEC_INSTALL_DIR} )
install(TARGETS fontinst_helper DESTINATION ${LIBEXEC_INSTALL_DIR} )
install(PROGRAMS fontinst_x11 DESTINATION ${LIBEXEC_INSTALL_DIR})

configure_file(org.kde.fontinst.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.fontinst.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.fontinst.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR} )

kde4_install_auth_helper_files(fontinst_helper org.kde.fontinst root)
kde4_install_auth_actions(org.kde.fontinst fontinst.actions)
