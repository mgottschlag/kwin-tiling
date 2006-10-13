


########### next target ###############

set(kwebdesktop_SRCS kwebdesktop.cpp )

kde4_automoc(kwebdesktop ${kwebdesktop_SRCS})

kde4_add_kcfg_files(kwebdesktop_SRCS kwebdesktopsettings.kcfgc )

kde4_add_executable(kwebdesktop ${kwebdesktop_SRCS})

target_link_libraries(kwebdesktop  ${KDE4_KPARTS_LIBS} ${KDE4_KHTML_LIBS} )

install(TARGETS kwebdesktop DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES kwebdesktop.kcfg   DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kwebdesktop.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kdesktop/programs )

