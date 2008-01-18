
add_subdirectory( pics )
#KDE4_NO_ENABLE_FINAL(background)
# Currently needed for DEFAULT_TEXT_HEIGHT
# The plan is to move konq_iconviewwidget to kdesktop and then it's all inside workspace.
# But first KonqOperations must be redesigned to not use konq_iconviewwidget...
include_directories( ${BLITZ_INCLUDES} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############
kde4_add_ui3_files(kcmbackgroundlib_SRCS
    bgdialog_ui.ui
    bgwallpaper_ui.ui
    bgadvanced_ui.ui
)

install( FILES background.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

#kde4_install_icons( ${ICON_INSTALL_DIR}  )

