set(ktip_SRCS ktipwindow.cpp)

kde4_add_executable(ktip ${ktip_SRCS})

target_link_libraries(ktip ${KDE4_KDEUI_LIBS})

install(TARGETS ktip DESTINATION ${BIN_INSTALL_DIR})

install(FILES ktip.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ktip.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES tips DESTINATION ${DATA_INSTALL_DIR}/kdewizard)
