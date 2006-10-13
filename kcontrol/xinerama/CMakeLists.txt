include_directories( ${X11_Xinerama_INCLUDE_PATH}   )


########### next target ###############

set(kcm_xinerama_PART_SRCS kcmxinerama.cpp )

kde4_automoc(kcm_xinerama ${kcm_xinerama_PART_SRCS})

kde4_add_ui3_files(kcm_xinerama_PART_SRCS xineramawidget.ui )

kde4_add_plugin(kcm_xinerama ${kcm_xinerama_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_xinerama )

target_link_libraries(kcm_xinerama  ${KDE4_KDE3SUPPORT_LIBS} ${X11_Xinerama_LIB})

install(TARGETS kcm_xinerama  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

set(test_kcm_xinerama_SRCS test_kcm_xinerama.cpp )
add_executable(test_kcm_xinerama ${test_kcm_xinerama_SRCS})
target_link_libraries(test_kcm_xinerama  ${QT_QTGUI_LIBRARY})
install(TARGETS test_kcm_xinerama  DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

install( FILES xinerama.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
