# Project Needs a name ofcourse
project(desktoptheme)

set(kcmdesktoptheme_SRCS kcmdesktoptheme.cpp desktopthemedetails.cpp thememodel.cpp)
kde4_add_ui_files(kcmdesktoptheme_SRCS DesktopTheme.ui DesktopThemeDetails.ui)

kde4_add_plugin(kcm_desktoptheme ${kcmdesktoptheme_SRCS})
target_link_libraries(kcm_desktoptheme ${QIMAGEBLITZ_LIBRARIES} ${X11_LIBRARIES}
    ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF3_LIBS}
    ${KDE4_KCMUTILS_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kcm_desktoptheme DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES desktoptheme.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

