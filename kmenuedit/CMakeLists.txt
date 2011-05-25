add_subdirectory( pixmaps ) 

set(kmenueditcommon_STAT_SRCS preferencesdlg.cpp klinespellchecking.cpp basictab.cpp treeview.cpp kmenuedit.cpp menufile.cpp menuinfo.cpp )
if(NOT WIN32)
set(kmenueditcommon_STAT_SRCS ${kmenueditcommon_STAT_SRCS} khotkeys.cpp)
endif(NOT WIN32)
########### next target ###############

qt4_add_dbus_adaptor( kmenueditcommon_STAT_SRCS org.kde.kmenuedit.xml kmenuedit.h KMenuEdit)

if(NOT WIN32)
qt4_add_dbus_interface(
    kmenueditcommon_STAT_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/../khotkeys/app/org.kde.khotkeys.xml
    khotkeys_interface)
endif(NOT WIN32)

set(kmenuedit_KDEINIT_SRCS main.cpp ${kmenueditcommon_STAT_SRCS})

kde4_add_kdeinit_executable( kmenuedit ${kmenuedit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kmenuedit ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTDBUS_LIBRARY})

install(TARGETS kdeinit_kmenuedit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmenuedit kdeinit_kmenuedit )
install(TARGETS kmenuedit ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS kmenuedit.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmenueditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmenuedit )

if(NOT WIN32)
add_dependencies(kdeinit_kmenuedit kded_khotkeys)
endif(NOT WIN32)

kde4_install_icons( ${ICON_INSTALL_DIR}  )

