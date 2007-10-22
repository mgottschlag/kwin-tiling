
project(kstyle-web)



########### next target ###############

set(webstyle_LIB_SRCS webstyle.cpp plugin.cpp )


kde4_add_plugin(webstyle ${webstyle_LIB_SRCS})

target_link_libraries(webstyle  ${KDE4_KDEUI_LIBS} )

install(TARGETS webstyle  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles )


########### install files ###############

install( FILES web.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )




