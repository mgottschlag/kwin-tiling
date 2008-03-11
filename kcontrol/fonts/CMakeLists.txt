
KDE4_NO_ENABLE_FINAL(fonts)
include_directories(${FONTCONFIG_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIR})

########### next target ###############

set(kcm_fonts_PART_SRCS ../krdb/krdb.cpp fonts.cpp ${libkxftconfig_SRCS} )


kde4_add_plugin(kcm_fonts ${kcm_fonts_PART_SRCS})


target_link_libraries(kcm_fonts  ${KDE4_KDEUI_LIBS} ${FONTCONFIG_LIBRARIES} ${FREETYPE_LIBRARIES})

install(TARGETS kcm_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES fonts.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
