


########### next target ###############

set(kcm_icons_PART_SRCS iconthemes.cpp icons.cpp main.cpp )

kde4_automoc(${kcm_icons_PART_SRCS})

kde4_add_plugin(kcm_icons ${kcm_icons_PART_SRCS})


target_link_libraries(kcm_icons  ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_icons  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############
if(KDE4_BUILD_TESTS)
set(testicons_SRCS testicons.cpp icons.cpp )

kde4_automoc(${testicons_SRCS})


kde4_add_executable(testicons ${testicons_SRCS})

target_link_libraries(testicons  ${KDE4_KDEUI_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES icons.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

