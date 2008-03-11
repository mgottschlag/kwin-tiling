add_subdirectory( background )

# Currently needed for DEFAULT_TEXT_HEIGHT
# The plan is to move konq_iconviewwidget to kdesktop and then it's all inside workspace.
# But first KonqOperations must be redesigned to not use konq_iconviewwidget...
include_directories( ${BLITZ_INCLUDES} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/background/  )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

kde4_add_ui_files(kcmbackgroundlib_SRCS ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kdm/background/bgwallpaper_ui.ui ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kdm/background/bgdialog_ui.ui ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kdm/background/bgadvanced_ui.ui)

set(kcm_kdm_PART_SRCS ${kcmbackgroundlib_SRCS} ${backgroundlib_SRCS}
   background.cpp
   kdm-gen.cpp
   kdm-dlg.cpp
   kdm-theme.cpp
   kdm-shut.cpp
   kdm-users.cpp
   kdm-conv.cpp
   main.cpp
   kbackedcombobox.cpp
   positioner.cpp)


kde4_add_plugin(kcm_kdm ${kcm_kdm_PART_SRCS})


target_link_libraries(kcm_kdm ${KDE4_KDE3SUPPORT_LIBS} ${BLITZ_LIBRARIES} ${KDE4_KNEWSTUFF2_LIBS})

install(TARGETS kcm_kdm  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kdm.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

install( FILES anchor.png  DESTINATION  ${DATA_INSTALL_DIR}/kcontrol/pics )
install( FILES kdm.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
