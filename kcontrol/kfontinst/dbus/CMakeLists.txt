include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/lib
                    ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}
                    ${POLKITQT_INCLUDE_DIR}
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/fonts)
add_definitions(${QT_DEFINITIONS})

set(fontinst_bin_SRCS FcConfig.cpp FontInst.cpp Folder.cpp Main.cpp Utils.cpp ${libkfontinstdbusiface_SRCS} )

# qt4_generate_dbus_interface(FontInst.h org.kde.fontinst.xml)
qt4_add_dbus_adaptor(fontinst_bin_SRCS org.kde.fontinst.xml FontInst.h KFI::FontInst)
# qt4_add_dbus_interface(fontinst_bin_SRCS org.kde.fontinst.xml FontinstIface)

kde4_add_executable(fontinst_bin ${fontinst_bin_SRCS})

set_target_properties(fontinst_bin PROPERTIES OUTPUT_NAME fontinst)
target_link_libraries(fontinst_bin ${POLKITQT_CORE_LIBRARY} ${KDE4_KDECORE_LIBS}
                      ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${FONTCONFIG_LIBRARIES} kfontinst)

install(TARGETS fontinst_bin DESTINATION ${LIBEXEC_INSTALL_DIR} )
install(PROGRAMS fontinst_x11 DESTINATION ${LIBEXEC_INSTALL_DIR})

configure_file(org.kde.fontinst.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.fontinst.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.fontinst.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR} )

# Use KAuth macros to create and install system dbus and policykit files
# Because I'm not using KAuth, but Polkit directly - org.kde.auth.conf needs 'policy context="default"' settings
# So, for the moment, dont use KAurth macro...
# kde4_install_auth_helper_files(fontinst org.kde.fontinst root)

install( FILES org.kde.fontinst.conf
         DESTINATION ${SYSCONF_INSTALL_DIR}/dbus-1/system.d )
configure_file(org.kde.fontinst.system-service.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.fontinst.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.fontinst.service DESTINATION ${DBUS_SYSTEM_SERVICES_INSTALL_DIR} )

kde4_install_auth_actions(org.kde.fontinst fontinst.actions)
