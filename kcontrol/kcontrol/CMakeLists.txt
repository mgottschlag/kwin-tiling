
add_subdirectory( about ) 



########### next target ###############

set(kcontrol_KDEINIT_SRCS 
   main.cpp 
   toplevel.cpp 
   indexwidget.cpp 
   helpwidget.cpp 
   dockcontainer.cpp 
   aboutwidget.cpp 
   quickhelp.cpp 
   moduletreeview.cpp 
   moduleiconview.cpp 
   moduleIface.cpp 
   global.cpp 
   modules.cpp 
   proxywidget.cpp 
   kcrootonly.cpp 
   searchwidget.cpp )

kde4_automoc(${kcontrol_KDEINIT_SRCS})


kde4_add_kdeinit_executable( kcontrol ${kcontrol_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcontrol  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KHTML_LIBS} )

install(TARGETS kdeinit_kcontrol  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kcontrol kdeinit_kcontrol )
install(TARGETS kcontrol DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install( FILES KControl.desktop kinfocenter.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kcontrolui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kcontrol )
install( FILES kinfocenterui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kinfocenter )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

