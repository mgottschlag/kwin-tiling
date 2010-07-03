KDE4_NO_ENABLE_FINAL(styles)

include_directories( ${QIMAGEBLITZ_INCLUDES} )

########### next target ###############

set(kcm_style_PART_SRCS ../krdb/krdb.cpp styleconfdialog.cpp kcmstyle.cpp)


kde4_add_ui_files(kcm_style_PART_SRCS stylepreview.ui finetuning.ui)

kde4_add_plugin(kcm_style ${kcm_style_PART_SRCS})

target_link_libraries(kcm_style ${KDE4_KIO_LIBS} ${QIMAGEBLITZ_LIBRARIES}
    ${X11_LIBRARIES} ${KDE4_PLASMA_LIBS} ${KDE4_KNEWSTUFF3_LIBS})

install(TARGETS kcm_style  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES style.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
