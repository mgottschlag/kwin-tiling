KDE4_NO_ENABLE_FINAL(styles)
add_subdirectory( keramik )

include_directories( ${BLITZ_INCLUDES} )

########### next target ###############

set(kcm_style_PART_SRCS ../krdb/krdb.cpp styleconfdialog.cpp kcmstyle.cpp)


kde4_add_ui_files(kcm_style_PART_SRCS stylepreview.ui)

kde4_add_plugin(kcm_style ${kcm_style_PART_SRCS})

target_link_libraries(kcm_style ${KDE4_KDEUI_LIBS} ${BLITZ_LIBRARIES})

install(TARGETS kcm_style  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES style.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
