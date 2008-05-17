
add_subdirectory( pics )
add_subdirectory( patterns )
add_subdirectory( programs )

include_directories( ${QIMAGEBLITZ_INCLUDES} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############
kde4_add_ui3_files(kcmbackgroundlib_SRCS
    bgdialog_ui.ui
    bgwallpaper_ui.ui
    bgadvanced_ui.ui
)

install( FILES background.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

#kde4_install_icons( ${ICON_INSTALL_DIR}  )

