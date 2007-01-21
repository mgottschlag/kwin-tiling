include_directories(
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/fonts/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/kcmfontinst/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/viewpart/
    )

set(kfontinst_bin_SRCS ${libkxftconfig_SRCS} ${libkfontinstupdatedialog_SRCS}
    Main.cpp Installer.cpp KfiPrint.cpp Viewer.cpp )

kde4_automoc(${kfontinst_bin_SRCS})

kde4_add_executable(kfontinst_bin ${kfontinst_bin_SRCS})
set_target_properties(kfontinst_bin PROPERTIES OUTPUT_NAME kfontinst)

target_link_libraries(kfontinst_bin  ${KDE4_KIO_LIBS} ${KDE4_KDEPRINT_LIBS} kfontinstui )

install(TARGETS kfontinst_bin DESTINATION ${BIN_INSTALL_DIR} )
install(FILES kfontinstui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kfontinst )
