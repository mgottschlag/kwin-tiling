
add_subdirectory( pics )
#KDE4_NO_ENABLE_FINAL(background)
# Currently needed for DEFAULT_TEXT_HEIGHT
# The plan is to move konq_iconviewwidget to kdesktop and then it's all inside workspace.
# But first KonqOperations must be redesigned to not use konq_iconviewwidget...
include_directories( ${BLITZ_INCLUDES} )
MESSAGE(STATUS "<${BLITZ_INCLUDES}>")
add_definitions (-DQT3_SUPPORT)

########### next target ###############
kde4_add_ui3_files(kcmbackgroundlib_SRCS
    bgdialog_ui.ui
    bgwallpaper_ui.ui
    bgadvanced_ui.ui
)

set(kcm_background_PART_SRCS main.cpp ${kcmbackgroundlib_SRCS} ${backgroundlib_SRCS})


kde4_add_plugin(kcm_background ${kcm_background_PART_SRCS})


target_link_libraries(kcm_background ${KDE4_KDE3SUPPORT_LIBS} ${BLITZ_LIBRARIES} ${KDE4_KNEWSTUFF2_LIBS} ${X11_X11_LIB})

install(TARGETS kcm_background  DESTINATION ${PLUGIN_INSTALL_DIR})



########### install files ###############

install( FILES background.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES background.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

