# Currently needed for DEFAULT_TEXT_HEIGHT
# The plan is to move konq_iconviewwidget to kdesktop and then it's all inside workspace.
# But first KonqOperations must be redesigned to not use konq_iconviewwidget...
include_directories( ${CMAKE_SOURCE_DIR}/libkonq )

add_definitions (-DQT3_SUPPORT)

kde4_add_ui3_files(kcmbackgroundlib_SRCS ${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgdialog_ui.ui ${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgwallpaper_ui.ui ${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgadvanced_ui.ui)


set(kcm_kdm_PART_SRCS ${kcmbackgroundlib_SRCS} ${backgroundlib_SRCS} 
   background.cpp 
   kdm-gen.cpp 
   kdm-dlg.cpp 
   kdm-theme.cpp 
   kdm-shut.cpp 
   kdm-users.cpp 
   kdm-conv.cpp 
   main.cpp
   klanguagebutton.cpp
   kbackedcombobox.cpp
   positioner.cpp)

kde4_automoc(${kcm_kdm_PART_SRCS})

kde4_add_plugin(kcm_kdm ${kcm_kdm_PART_SRCS})


target_link_libraries(kcm_kdm ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDEFX_LIBS} ${KDE4_KNEWSTUFF2_LIBS})

install(TARGETS kcm_kdm  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kdm.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

install( FILES anchor.png  DESTINATION  ${DATA_INSTALL_DIR}/kcontrol/pics )
