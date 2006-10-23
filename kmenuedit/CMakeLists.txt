
project(kmenuedit)

add_subdirectory( pixmaps ) 



set(kmenueditcommon_STAT_SRCS basictab.cpp treeview.cpp kmenuedit.cpp khotkeys.cpp menufile.cpp menuinfo.cpp )

########### next target ###############

set(kmenuedit_KDEINIT_SRCS main.cpp ${kmenueditcommon_STAT_SRCS})

kde4_automoc(${kmenuedit_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kmenuedit ${kmenuedit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kmenuedit  ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTXML_LIBRARY}
)

install(TARGETS kdeinit_kmenuedit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmenuedit kdeinit_kmenuedit )
install(TARGETS kmenuedit DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(kcontroledit_KDEINIT_SRCS kcontrol_main.cpp ${kmenueditcommon_STAT_SRCS})

kde4_automoc(${kcontroledit_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcontroledit ${kcontroledit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcontroledit  ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTXML_LIBRARY} )

install(TARGETS kdeinit_kcontroledit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kcontroledit kdeinit_kcontroledit )
install(TARGETS kcontroledit DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############


########### install files ###############

install( FILES kmenuedit.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kcontroleditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kcontroledit )
install( FILES kmenueditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmenuedit )


kde4_install_icons( ${ICON_INSTALL_DIR}  )

