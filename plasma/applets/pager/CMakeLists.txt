project(plasma-pager)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PLASMA_INCLUDE_DIR})

set(pager_SRCS
    pager.cpp)

kde4_add_ui_files(pager_SRCS pagerConfig.ui )
kde4_add_plugin(plasma_applet_pager ${pager_SRCS})
target_link_libraries(plasma_applet_pager ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_pager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-pager-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})


