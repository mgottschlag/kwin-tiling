
add_subdirectory( keramik )

########### next target ###############

set(kcm_style_PART_SRCS ${libkrd_SRCS} menupreview.cpp styleconfdialog.cpp kcmstyle.cpp)

kde4_automoc(${kcm_style_PART_SRCS})

kde4_add_ui_files(kcm_style_PART_SRCS stylepreview.ui)

kde4_add_plugin(kcm_style ${kcm_style_PART_SRCS})

target_link_libraries(kcm_style ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDEFX_LIBS})

install(TARGETS kcm_style  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES style.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )
