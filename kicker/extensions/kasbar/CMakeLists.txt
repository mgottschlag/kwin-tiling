


########### next target ###############

set(kasbar_LIB_SRCS 
   kasbar.cpp 
   kasitem.cpp 
   kastaskitem.cpp 
   kastasker.cpp 
   kaspopup.cpp 
   kastaskpopup.cpp 
   kasstartupitem.cpp 
   kasaboutdlg.cpp 
   kasprefsdlg.cpp 
   kasclockitem.cpp 
   kasgroupitem.cpp 
   kasresources.cpp 
   kasgrouper.cpp 
   kasloaditem.cpp )

kde4_automoc(${kasbar_LIB_SRCS})

kde4_add_library(kasbar SHARED ${kasbar_LIB_SRCS})

target_link_libraries(kasbar  ${KDE4_KDEUI_LIBS} taskmanager kasbar ${KDE4_KDEUI_LIBS} )

set_target_properties(kasbar PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kasbar  DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

set(kasbar_panelextension_PART_SRCS kasbarextension.cpp )

kde4_automoc(${kasbar_panelextension_PART_SRCS})

kde4_add_plugin(kasbar_panelextension ${kasbar_panelextension_PART_SRCS})


target_link_libraries(kasbar_panelextension  ${KDE4_KDECORE_LIBS} kasbar )

install(TARGETS kasbar_panelextension  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kasbar_SRCS kasbarapp.cpp )

kde4_automoc(${kasbar_SRCS})

kde4_add_executable(kasbar ${kasbar_SRCS})

target_link_libraries(kasbar  ${KDE4_KDECORE_LIBS} )

install(TARGETS kasbar DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES kasbarextension.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/extensions )


