
configure_file (config-screensaver.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-screensaver.h )

########### next target ###############

set(kcm_screensaver_PART_SRCS 
   scrnsave.cpp 
   testwin.cpp 
   saverlist.cpp 
   saverconfig.cpp 
   advanceddialog.cpp 
   kswidget.cpp )

kde4_automoc(${kcm_screensaver_PART_SRCS})

kde4_add_ui3_files(kcm_screensaver_PART_SRCS advanceddialogimpl.ui )

kde4_add_plugin(kcm_screensaver ${kcm_screensaver_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_screensaver )

target_link_libraries(kcm_screensaver  ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcm_screensaver  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES screensaver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

