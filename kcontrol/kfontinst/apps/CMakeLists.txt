include_directories(
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/fonts/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/kcmfontinst/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/viewpart/
    )

set(kfontinst_bin_SRCS ${libkfontinstjobrunner_SRCS} Installer.cpp )
set(kfontprint_bin_SRCS KfiPrint.cpp )
set(kfontview_bin_SRCS Viewer.cpp )

kde4_automoc(${kfontinst_bin_SRCS} ${kfontprint_bin_SRCS} ${kfontview_bin_SRCS} )

kde4_add_executable(kfontinst_bin ${kfontinst_bin_SRCS})
kde4_add_executable(kfontprint_bin ${kfontprint_bin_SRCS})
kde4_add_executable(kfontview_bin ${kfontview_bin_SRCS})

set_target_properties(kfontinst_bin PROPERTIES OUTPUT_NAME kfontinst)
set_target_properties(kfontprint_bin PROPERTIES OUTPUT_NAME kfontprint)
set_target_properties(kfontview_bin PROPERTIES OUTPUT_NAME kfontview)

target_link_libraries(kfontinst_bin ${KDE4_KIO_LIBS} kfontinst )
target_link_libraries(kfontprint_bin ${KDE4_KDEPRINT_LIBS} kfontinstui kfontinst )
target_link_libraries(kfontview_bin ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS} kfontinstui kfontinst )

install(TARGETS kfontinst_bin DESTINATION ${BIN_INSTALL_DIR} )
install(TARGETS kfontprint_bin DESTINATION ${DATA_INSTALL_DIR}/kfontinst/bin )
install(TARGETS kfontview_bin DESTINATION ${BIN_INSTALL_DIR} )
install(FILES kfontinstui.rc DESTINATION ${DATA_INSTALL_DIR}/kfontinst )
install(FILES kfontview.desktop DESTINATION ${XDG_APPS_DIR} )
install(FILES installfont.desktop DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus )

kde4_install_icons( ${ICON_INSTALL_DIR} )
