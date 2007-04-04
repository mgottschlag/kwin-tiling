include_directories( ${X11_Xinerama_INCLUDE_PATH}   )

add_definitions (-DQT3_SUPPORT)

########### next target ###############
set(kcm_xinerama_kwin_SRCS)


set(kwin_xml  ${CMAKE_SOURCE_DIR}/workspace/kwin/org.kde.KWin.xml)

QT4_ADD_DBUS_INTERFACE( kcm_xinerama_kwin_SRCS ${kwin_xml} kwin_interface )

set(kcm_xinerama_PART_SRCS kcmxinerama.cpp xineramawidget.cpp ${kcm_xinerama_kwin_SRCS})


kde4_automoc(${kcm_xinerama_PART_SRCS})

kde4_add_ui_files(kcm_xinerama_PART_SRCS xineramawidget.ui )

kde4_add_plugin(kcm_xinerama ${kcm_xinerama_PART_SRCS})


target_link_libraries(kcm_xinerama  ${KDE4_KDE3SUPPORT_LIBS} ${X11_Xinerama_LIB})

install(TARGETS kcm_xinerama  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

set(test_kcm_xinerama_SRCS test_kcm_xinerama.cpp ${kcm_xinerama_kwin_SRCS} )
add_executable(test_kcm_xinerama ${test_kcm_xinerama_SRCS})
target_link_libraries(test_kcm_xinerama  ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY})
if (CMAKE_SYSTEM_NAME MATCHES BSD)
    target_link_libraries(test_kcm_xinerama pthread)
endif (CMAKE_SYSTEM_NAME MATCHES BSD)
install(TARGETS test_kcm_xinerama  DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

install( FILES xinerama.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
