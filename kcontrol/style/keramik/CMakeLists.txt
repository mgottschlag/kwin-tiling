


########### next target ###############

set(kstyle_keramik_config_PART_SRCS keramikconf.cpp )

kde4_automoc(${kstyle_keramik_config_PART_SRCS})

kde4_add_plugin(kstyle_keramik_config ${kstyle_keramik_config_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kstyle_keramik_config )

target_link_libraries(kstyle_keramik_config  ${KDE4_KDEUI_LIBS} )

install(TARGETS kstyle_keramik_config  DESTINATION ${PLUGIN_INSTALL_DIR} )


