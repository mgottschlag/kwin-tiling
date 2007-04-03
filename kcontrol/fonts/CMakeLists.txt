

include_directories(${FONTCONFIG_INCLUDE_DIR})

########### next target ###############

set(kcm_fonts_PART_SRCS ${libkrd_SRCS} fonts.cpp ${libkxftconfig_SRCS} )

kde4_automoc(${kcm_fonts_PART_SRCS})

kde4_add_plugin(kcm_fonts ${kcm_fonts_PART_SRCS})


target_link_libraries(kcm_fonts  ${KDE4_KDEUI_LIBS} ${FONTCONFIG_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES fonts.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

