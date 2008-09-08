
include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/fonts
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/kcmfontinst
                    ${CMAKE_CURRENT_BINARY_DIR})

# kio_fonts_helper can use PolicyKit if this is installed...
if (KFI_USE_POLICY_KIT AND POLKITDBUS_FOUND)

    set(kio_fonts_PART_SRCS KioFonts.cpp ${libkxftconfig_SRCS} ${kfi_polkit_auth_SRCS})

    qt4_add_dbus_interface(kio_fonts_PART_SRCS org.kde.fontinst.xml FontInstInterface )

    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} polkit --variable=policydir
                    OUTPUT_VARIABLE POLICY_DIR)
    string(REGEX REPLACE "[\r\n]" "" POLICY_DIR "${POLICY_DIR}")

    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} dbus-1 --variable=sysconfdir
                    OUTPUT_VARIABLE DBUS_SYSCONF_DIR)
    string(REGEX REPLACE "[\r\n]" "" DBUS_SYSCONF_DIR "${DBUS_SYSCONF_DIR}")

    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} dbus-1 --variable=session_bus_services_dir
                    OUTPUT_VARIABLE DBUS_SESSION_SRV_DIR)
    string(REGEX REPLACE "[\r\n]" "" DBUS_SESSION_SRV_DIR "${DBUS_SESSION_SRV_DIR}")

    include_directories (${CMAKE_SOURCE_DIR}
                         ${CMAKE_CURRENT_BINARY_DIR}
                         ${CMAKE_BINARY_DIR}
                         ${QT_INCLUDE_DIR}
                         ${KFI_POLKIT_INCLUDES} )

    set(kio_fonts_helper_SRCS FontInst.cpp PolicyKitMechanism.cpp ${libkxftconfig_SRCS})

    # qdbuscpp2xml -M -S FontInst.h -o org.kde.fontinst.xml

    qt4_add_dbus_adaptor(kio_fonts_helper_SRCS org.kde.fontinst.xml FontInst.h FontInst)

    kde4_add_executable(kio_fonts_helper_bin ${kio_fonts_helper_SRCS})
    set_target_properties(kio_fonts_helper_bin PROPERTIES OUTPUT_NAME kio_fonts_helper)

    target_link_libraries(kio_fonts_helper_bin ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY} kfontinst ${KFI_POLKIT_LIBS})
    install(TARGETS kio_fonts_helper_bin DESTINATION ${LIBEXEC_INSTALL_DIR})

    configure_file(org.kde.fontinst.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.fontinst.service @ONLY)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.fontinst.service
            DESTINATION ${DBUS_SESSION_SRV_DIR}/../system-services/)
    install(FILES org.kde.fontinst.conf
            DESTINATION ${DBUS_SYSCONF_DIR}/dbus-1/system.d/)
    install(FILES org.kde.fontinst.policy
            DESTINATION ${POLICY_DIR})

else (KFI_USE_POLICY_KIT AND POLKITDBUS_FOUND)

    set(kio_fonts_PART_SRCS KioFonts.cpp Server.cpp Socket.cpp SuProc.cpp ${libkxftconfig_SRCS})
    set(kio_fonts_helper_SRCS Helper.cpp Socket.cpp ${libkxftconfig_SRCS})
    kde4_add_executable(kio_fonts_helper ${kio_fonts_helper_SRCS} )
    target_link_libraries(kio_fonts_helper ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} kfontinst )
    install(TARGETS kio_fonts_helper DESTINATION ${LIBEXEC_INSTALL_DIR} )

endif (KFI_USE_POLICY_KIT AND POLKITDBUS_FOUND)

kde4_add_plugin(kio_fonts ${kio_fonts_PART_SRCS} ${KFI_FONTINST_AUTH_SRC} )
target_link_libraries(kio_fonts ${KDE4_KIO_LIBS} ${KDE4_KDESU_LIBS} ${QT_QTNETWORK_LIBRARY} ${FONTCONFIG_LIBRARIES} ${FREETYPE_LIBRARIES} kfontinst )
install(TARGETS kio_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES fonts.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services )
install( FILES fonts.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
kde4_install_icons( ${ICON_INSTALL_DIR}  )

