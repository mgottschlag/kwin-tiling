
project(kdmlib)
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kdm/kfrontend  )

########### next target ###############

set(kgreet_classic_PART_SRCS kgreet_classic.cpp )

kde4_automoc(kgreet_classic ${kgreet_classic_PART_SRCS})

kde4_add_plugin(kgreet_classic ${kgreet_classic_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kgreet_classic )

target_link_libraries(kgreet_classic  ${KDE4_KDEUI_LIBS})

install(TARGETS kgreet_classic  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kgreet_winbind_PART_SRCS kgreet_winbind.cpp )

kde4_automoc(kgreet_winbind ${kgreet_winbind_PART_SRCS})

kde4_add_plugin(kgreet_winbind ${kgreet_winbind_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kgreet_winbind )

target_link_libraries(kgreet_winbind  ${KDE4_KDEUI_LIBS})

install(TARGETS kgreet_winbind  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kgreeterplugin.h DESTINATION ${INCLUDE_INSTALL_DIR} )
