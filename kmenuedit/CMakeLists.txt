add_subdirectory( pixmaps ) 

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(kmenueditcommon_STAT_SRCS preferencesdlg.cpp klinespellchecking.cpp basictab.cpp treeview.cpp kmenuedit.cpp khotkeys.cpp menufile.cpp menuinfo.cpp )

########### next target ###############

qt4_add_dbus_adaptor( kmenueditcommon_STAT_SRCS org.kde.kmenuedit.xml kmenuedit.h KMenuEdit)

set(kmenuedit_KDEINIT_SRCS main.cpp ${kmenueditcommon_STAT_SRCS})

kde4_add_kdeinit_executable( kmenuedit ${kmenuedit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kmenuedit  ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTXML_LIBRARY}
)

install(TARGETS kdeinit_kmenuedit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmenuedit kdeinit_kmenuedit )
install(TARGETS kmenuedit DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install( FILES kmenuedit.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmenueditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmenuedit )


kde4_install_icons( ${ICON_INSTALL_DIR}  )

