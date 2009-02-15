
add_subdirectory( pics )
add_subdirectory( patterns )
add_subdirectory( programs )

include_directories( ${QIMAGEBLITZ_INCLUDES} )

########### next target ###############
kde4_add_ui_files(kcmbackgroundlib_SRCS
    bgdialog_ui.ui
    bgwallpaper_ui.ui
    bgadvanced_ui.ui
)

install( FILES background.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

#kde4_install_icons( ${ICON_INSTALL_DIR}  )

