# Project Needs a name ofcourse
project(desktopthemedetails)
 
set(kcmdesktopthemedetails_SRCS desktopthemedetails.cpp)
kde4_add_ui_files(kcmdesktopthemedetails_SRCS DesktopThemeDetails.ui)

kde4_add_plugin(kcm_desktopthemedetails ${kcmdesktopthemedetails_SRCS})
target_link_libraries(kcm_desktopthemedetails ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KUTILS_LIBRARY} plasma ${KDE4_KIO_LIBS})

install(TARGETS kcm_desktopthemedetails  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES desktopthemedetails.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES themeitems DESTINATION ${DATA_INSTALL_DIR}/desktopthemedetails )
