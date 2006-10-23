
add_subdirectory( pics ) 
include_directories( ${CMAKE_SOURCE_DIR}/libkonq   )


########### next target ###############
kde4_add_ui3_files(kcmbackgroundlib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/bgdialog_ui.ui ${CMAKE_CURRENT_SOURCE_DIR}/bgwallpaper_ui.ui ${CMAKE_CURRENT_SOURCE_DIR}/bgadvanced_ui.ui )


set(kcm_background_PART_SRCS main.cpp ${kcmbackgroundlib_SRCS} ${backgroundlib_SRCS})

kde4_automoc(${kcm_background_PART_SRCS})

kde4_add_plugin(kcm_background ${kcm_background_PART_SRCS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcm_background)

target_link_libraries(kcm_background ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDEFX_LIBS} ${KDE4_KNEWSTUFF_LIBS} ${X11_X11_LIB})

install(TARGETS kcm_background  DESTINATION ${PLUGIN_INSTALL_DIR})



########### install files ###############

install( FILES background.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

