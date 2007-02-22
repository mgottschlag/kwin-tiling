
project(kdesktop)

configure_file(config-kdesktop.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdesktop.h)

# used by kdesktop but also by lock/
set( kdesktop_KCFG_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/kdesktopsettings.kcfgc
	${CMAKE_CURRENT_SOURCE_DIR}/klaunchsettings.kcfgc )

kde4_add_kcfg_files(kdesktop_LIB_SRCS ${kdesktop_KCFG_SRCS})
include_directories(
	${CMAKE_SOURCE_DIR}/workspace/lib
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background
	${CMAKE_SOURCE_DIR}/libkonq
	${CMAKE_SOURCE_DIR}/workspace/kdmlib
	 )


########### next target ###############

# the only reason to use kde4_add_executable() instead of 
# the plain add_executable() call here is the RPATH handling
# in kde4_add_executable, Alex
kde4_add_executable(kcheckrunning kcheckrunning.cpp)

target_link_libraries(kcheckrunning  ${X11_LIBRARIES} )

install(TARGETS kcheckrunning DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############


# Please use .cpp for new files
set(kdesktop_SRCS ${kdesktop_LIB_SRCS} ${kdmlib_SRCS} ${backgroundlib_SRCS}
   main.cc
   krootwm.cc
   kdiconview.cc
   desktop.cc
   bgmanager.cc
   init.cc
   minicli.cpp
   pixmapserver.cc
   kcustommenu.cc
   startupid.cpp
   kshadowengine.cpp
   kshadowsettings.cpp
   kdesktopshadowsettings.cpp
   kfileividesktop.cpp
)

set(ksmserver_xml  ${CMAKE_SOURCE_DIR}/workspace/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE( kdesktop_SRCS ${ksmserver_xml} ksmserver_interface )
set(kwin_xml  ${CMAKE_SOURCE_DIR}/workspace/kwin/org.kde.KWin.xml)
QT4_ADD_DBUS_INTERFACE( kdesktop_SRCS ${kwin_xml} kwin_interface )

qt4_add_dbus_adaptor( kdesktop_SRCS org.kde.kdesktop.Background.xml bgmanager.h KBackgroundManager )
qt4_add_dbus_adaptor( kdesktop_SRCS org.kde.kdesktop.Desktop.xml desktop.h KDesktop )

kde4_automoc(${kdesktop_SRCS})

kde4_add_ui_files(kdesktop_SRCS minicli.ui )


kde4_add_kdeinit_executable( kdesktop ${kdesktop_SRCS})

target_link_libraries(kdeinit_kdesktop  ${KDE4_KDE3SUPPORT_LIBS} kworkspace ${KDE4_KDESU_LIBS} konq ${X11_X11_LIB} ${X11_Xext_LIB})
if(X11_Xcursor_FOUND)
  target_link_libraries(kdeinit_kdesktop ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)
if(X11_Xss_LIB)
  target_link_libraries(kdeinit_kdesktop ${X11_Xss_LIB})
endif(X11_Xss_LIB)

install(TARGETS kdeinit_kdesktop  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kdesktop kdeinit_kdesktop )
install(TARGETS kdesktop DESTINATION ${BIN_INSTALL_DIR})

########### subdirs ###############

add_subdirectory( patterns )
add_subdirectory( programs )
add_subdirectory( init )
add_subdirectory( kwebdesktop )

########### install files ###############

install( FILES kdesktop.kcfg klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
#install( FILES kdesktop.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
install( FILES kdesktopSetAsBackground.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/servicemenus )
install( FILES kdesktop_custom_menu1 kdesktop_custom_menu2  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES org.kde.kdesktop.Background.xml org.kde.kdesktop.Desktop.xml DESTINATION ${DBUS_INTERFACES_DIR} )



