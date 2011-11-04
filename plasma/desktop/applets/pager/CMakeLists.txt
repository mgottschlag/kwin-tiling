project(plasma-pager)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)

set(pager_SRCS
    pager.cpp)

kde4_add_ui_files(pager_SRCS pagerConfig.ui )
kde4_add_plugin(plasma_applet_pager ${pager_SRCS})
target_link_libraries(plasma_applet_pager ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_KCMUTILS_LIBRARY} ${KACTIVITIES_LIBRARY} kephal taskmanager)

install(TARGETS plasma_applet_pager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-pager-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})


